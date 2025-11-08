#!/usr/bin/env python3
"""
System Review Generator
주간/월간 시스템 리뷰를 생성합니다.
"""

import argparse
import subprocess
import json
import re
import datetime
from pathlib import Path
from collections import defaultdict, Counter
from jinja2 import Template

def sh(cmd):
    """셸 명령 실행"""
    try:
        return subprocess.check_output(cmd, shell=True, text=True, errors="ignore").strip()
    except subprocess.CalledProcessError:
        return ""

def get_period_range(period_type, date_str=None):
    """기간 범위 계산"""
    if date_str:
        target = datetime.datetime.strptime(date_str, "%Y-%m-%d").date()
    else:
        target = datetime.date.today()

    if period_type == "weekly":
        # 월요일 ~ 일요일
        monday = target - datetime.timedelta(days=target.weekday())
        end_date = monday + datetime.timedelta(days=6)
        period_label = f"{monday.year}-W{monday.isocalendar()[1]:02d}"
        analysis_days = 7
    else:  # monthly
        # 월 첫날 ~ 마지막날
        first_day = target.replace(day=1)
        if target.month == 12:
            last_day = target.replace(month=12, day=31)
        else:
            next_month = target.replace(month=target.month + 1, day=1)
            last_day = next_month - datetime.timedelta(days=1)

        period_label = f"{target.year}-{target.month:02d}"
        analysis_days = (last_day - first_day).days + 1
        monday = first_day
        end_date = last_day

    return {
        "start": monday,
        "end": end_date,
        "label": period_label,
        "type": period_type,
        "days": analysis_days
    }

def git_log_range(since, until):
    """특정 기간의 커밋 해시 목록"""
    result = sh(f'git log --since="{since}" --until="{until}" --pretty=format:"%H"')
    return result.splitlines() if result else []

def analyze_hotspots(commits):
    """Hotspot 파일 분석"""
    file_stats = defaultdict(lambda: {"freq": 0, "lines": 0, "commits": []})

    for commit_hash in commits:
        numstat = sh(f'git show --numstat --format="" {commit_hash}')
        for ln in numstat.splitlines():
            m = re.match(r"(\d+|-)\s+(\d+|-)\s+(.+)", ln)
            if not m:
                continue
            a, d, f = m.groups()

            # 바이너리나 삭제된 파일 제외
            if a == '-' or d == '-':
                continue

            lines = int(a) + int(d)
            file_stats[f]["freq"] += 1
            file_stats[f]["lines"] += lines
            file_stats[f]["commits"].append(commit_hash[:7])

    # 위험도 계산 (변경 빈도 + 변경량)
    hotspots = []
    for path, stats in file_stats.items():
        risk_score = stats["freq"] * 2 + (stats["lines"] / 100)

        if risk_score > 10:
            risk_level = "🔴 High"
        elif risk_score > 5:
            risk_level = "🟡 Medium"
        else:
            risk_level = "🟢 Low"

        hotspots.append({
            "path": path,
            "frequency": stats["freq"],
            "lines": stats["lines"],
            "complexity": "N/A",  # TODO: Lizard 연동
            "risk_level": risk_level,
            "risk_score": risk_score
        })

    # 위험도 순 정렬
    hotspots.sort(key=lambda x: x["risk_score"], reverse=True)

    return hotspots[:10]

def analyze_api_changes(devlog_dir, period):
    """Doxygen 변화 분석"""
    # TODO: Doxygen JSON diff 구현
    # 현재는 플레이스홀더
    return {
        "added": 0,
        "changed": 0,
        "removed": 0,
        "notable": []
    }

def load_weekly_logs(devlog_dir, start_date, end_date):
    """주간 리포트 로드"""
    devlog_path = Path(devlog_dir)
    weekly_logs = []

    # 주차별로 탐색
    current = start_date
    while current <= end_date:
        week_num = current.isocalendar()[1]
        year = current.year
        week_label = f"{year}-W{week_num:02d}"

        log_file = devlog_path / f"{week_label}-Summary.md"
        if log_file.exists():
            weekly_logs.append(week_label)

        current += datetime.timedelta(days=7)

    return weekly_logs

def generate_hotspot_summary(hotspots):
    """Hotspot 요약 생성"""
    if not hotspots:
        return "Hotspot 파일이 감지되지 않았습니다."

    high_risk = [h for h in hotspots if "High" in h["risk_level"]]

    if high_risk:
        summary = f"{len(high_risk)}개의 고위험 파일이 감지되었습니다. "
        summary += f"특히 '{high_risk[0]['path']}'는 {high_risk[0]['frequency']}회 변경되었으며, "
        summary += "구조적 개선이 필요할 수 있습니다."
    else:
        summary = "대부분의 파일이 안정적으로 관리되고 있습니다. "
        summary += "특별한 Hotspot이 발견되지 않았습니다."

    return summary

def generate_overall_assessment(hotspots, quality_metrics, performance_metrics):
    """전반적 평가 생성"""
    assessment_parts = []

    # Hotspot 평가
    high_risk_count = len([h for h in hotspots if "High" in h.get("risk_level", "")])
    if high_risk_count > 3:
        assessment_parts.append("코드베이스에 여러 Hotspot이 감지되어 구조적 개선이 필요합니다.")
    elif high_risk_count > 0:
        assessment_parts.append("일부 Hotspot 파일이 있으나 관리 가능한 수준입니다.")
    else:
        assessment_parts.append("코드베이스가 안정적으로 관리되고 있습니다.")

    # 성능 평가 (데이터가 있을 경우)
    if performance_metrics:
        if performance_metrics.get("load_delta", 0) < -5:
            assessment_parts.append("로딩 성능이 크게 개선되었습니다.")
        elif performance_metrics.get("load_delta", 0) > 10:
            assessment_parts.append("로딩 성능 저하가 발생했으며 최적화가 필요합니다.")

    if not assessment_parts:
        return "이번 기간 동안 시스템은 전반적으로 안정적이었습니다."

    return " ".join(assessment_parts)

def render(context, template_path, out_path):
    """Jinja2 템플릿 렌더링"""
    template_content = Path(template_path).read_text(encoding="utf-8")
    md = Template(template_content).render(**context)

    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    Path(out_path).write_text(md, encoding="utf-8")

def main():
    ap = argparse.ArgumentParser(description="System Review Generator")
    ap.add_argument("--type", choices=["weekly", "monthly"], default="monthly", help="리뷰 타입")
    ap.add_argument("--date", default=None, help="기준 날짜 (YYYY-MM-DD)")
    ap.add_argument("--devlog-dir", default="Documents/DevLog", help="DevLog 디렉토리")
    ap.add_argument("--out-dir", default="Documents/SystemReview", help="출력 디렉토리")
    ap.add_argument("--template", default=None, help="템플릿 파일 경로")
    args = ap.parse_args()

    # 템플릿 경로 설정
    if not args.template:
        script_dir = Path(__file__).parent
        args.template = script_dir / "system_review_template.md"

    # 기간 계산
    period = get_period_range(args.type, args.date)
    start_date = period["start"]
    end_date = period["end"]
    period_label = period["label"]

    print(f"📊 Generating System Review: {period_label}")
    print(f"   Type: {args.type}")
    print(f"   Period: {start_date} ~ {end_date}")

    # Git 커밋 수집
    since = start_date.strftime("%Y-%m-%d 00:00:00")
    until = (end_date + datetime.timedelta(days=1)).strftime("%Y-%m-%d 00:00:00")

    commits = git_log_range(since, until)
    print(f"   Found {len(commits)} commits")

    # Hotspot 분석
    hotspots = analyze_hotspots(commits)
    hotspot_summary = generate_hotspot_summary(hotspots)

    # API 변화 분석
    api_changes = analyze_api_changes(args.devlog_dir, period)

    # Weekly Logs 수집
    weekly_logs = load_weekly_logs(args.devlog_dir, start_date, end_date)

    # 전반적 평가
    overall_assessment = generate_overall_assessment(hotspots, None, None)

    # 컨텍스트 구성
    ctx = {
        "period": period_label,
        "date_from": start_date.isoformat(),
        "date_to": end_date.isoformat(),
        "analysis_days": period["days"],
        "new_subsystems": [],  # TODO: 자동 감지
        "removed_modules": [],  # TODO: 자동 감지
        "api_changes": api_changes,
        "affected_areas": [],  # TODO: 영향 범위 분석
        "hotspot_analysis": hotspots,
        "hotspot_summary": hotspot_summary,
        "quality_metrics": None,  # TODO: 정적분석 연동
        "recurring_patterns": [],  # TODO: 패턴 감지
        "performance_metrics": None,  # TODO: 성능 메트릭 수집
        "stability_metrics": None,  # TODO: 안정성 메트릭
        "ue5_metrics": None,  # TODO: UE5 메트릭
        "feature_trend": None,  # TODO: 기능 트렌드 분석
        "refactor_trend": None,  # TODO: 리팩터링 트렌드
        "bug_trend": None,  # TODO: 버그 트렌드
        "risks": [],  # TODO: 리스크 자동 감지
        "tech_debt": [],  # TODO: 기술 부채 분석
        "tech_debt_score": 0,
        "urgent_actions": [],  # 사용자 작성
        "improvement_plans": [],  # 사용자 작성
        "test_improvements": [],  # 사용자 작성
        "subsystems": [],  # TODO: Doxygen에서 추출
        "dependencies": [],  # TODO: 의존성 그래프
        "changed_subsystems": [],  # TODO: 변경된 서브시스템
        "overall_assessment": overall_assessment,
        "strengths": [],  # 사용자 작성
        "improvements_needed": [],  # 사용자 작성
        "recommendations": [],  # 사용자 작성
        "weekly_logs": weekly_logs,
        "generation_time": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    }

    # 출력 파일 경로
    out_path = Path(args.out_dir) / f"{period_label}-SystemReview.md"

    # 렌더링 및 저장
    render(ctx, args.template, out_path)
    print(f"✅ System Review generated: {out_path}")

if __name__ == "__main__":
    main()
