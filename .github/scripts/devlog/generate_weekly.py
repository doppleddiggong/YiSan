#!/usr/bin/env python3
"""
Weekly DevLog Generator
7일간의 Daily DevLog를 분석하여 주간 리포트를 생성합니다.
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

def get_week_range(date_str=None):
    """주간 범위 계산 (월요일 ~ 일요일)"""
    if date_str:
        target = datetime.datetime.strptime(date_str, "%Y-%m-%d").date()
    else:
        target = datetime.date.today()

    # 월요일 찾기
    monday = target - datetime.timedelta(days=target.weekday())
    sunday = monday + datetime.timedelta(days=6)

    # ISO week number
    week_num = monday.isocalendar()[1]
    year = monday.year

    return {
        "monday": monday,
        "sunday": sunday,
        "week_label": f"{year}-W{week_num:02d}",
        "year": year,
        "week": week_num
    }

def git_log_range(since, until):
    """특정 기간의 커밋 해시 목록"""
    result = sh(f'git log --since="{since}" --until="{until}" --pretty=format:"%H"')
    return result.splitlines() if result else []

def parse_commit(commit_hash):
    """커밋 정보 파싱"""
    # 메타데이터
    meta = sh(f'git show --format="%s||%h||%an||%ae||%ai" --no-patch {commit_hash}')
    if not meta or "||" not in meta:
        return None

    parts = meta.split("||")
    if len(parts) < 5:
        return None

    subject, short, author, email, date = parts

    # 변경 파일 통계
    numstat = sh(f'git show --numstat --format="" {commit_hash}')
    added = deleted = 0
    files_changed = []

    for ln in numstat.splitlines():
        m = re.match(r"(\d+|-)\s+(\d+|-)\s+(.+)", ln)
        if not m:
            continue
        a, d, f = m.groups()
        if a != '-' and d != '-':
            added += int(a)
            deleted += int(d)
            files_changed.append((f, int(a) + int(d)))

    # Conventional Commits 파싱
    commit_type = parse_commit_type(subject)

    # PR 번호 추출
    pr_match = re.search(r"#(\d+)", subject)
    pr_number = pr_match.group(1) if pr_match else None

    return {
        "hash": commit_hash,
        "short": short,
        "subject": subject,
        "author": author,
        "email": email,
        "date": date,
        "type": commit_type,
        "pr": pr_number,
        "added": added,
        "deleted": deleted,
        "files_changed": files_changed
    }

def parse_commit_type(subject):
    """Conventional Commits에서 타입 추출"""
    match = re.match(r"^(feat|fix|docs|style|refactor|test|chore|perf)(\(.+?\))?:", subject.lower())
    if match:
        return match.group(1)
    return "other"

def analyze_commits(commits):
    """커밋 분석"""
    total_added = 0
    total_deleted = 0
    commit_types = Counter()
    authors = Counter()
    hotspot_files = defaultdict(lambda: {"lines": 0, "freq": 0})

    features = []
    fixes = []
    refactors = []
    performance = []

    active_days = set()

    for commit in commits:
        if not commit:
            continue

        total_added += commit["added"]
        total_deleted += commit["deleted"]
        commit_types[commit["type"]] += 1
        authors[commit["author"]] += 1

        # 날짜별 활동
        commit_date = commit["date"][:10]  # YYYY-MM-DD
        active_days.add(commit_date)

        # Hotspot 파일
        for f, lines in commit["files_changed"]:
            hotspot_files[f]["lines"] += lines
            hotspot_files[f]["freq"] += 1

        # 타입별 분류
        item = {
            "title": commit["subject"],
            "pr": commit["pr"],
            "description": f"by {commit['author']} on {commit['date'][:10]}"
        }

        if commit["type"] == "feat":
            features.append(item)
        elif commit["type"] == "fix":
            fixes.append(item)
        elif commit["type"] == "refactor":
            refactors.append(item)
        elif commit["type"] == "perf":
            performance.append(item)

    # Hotspot 파일 정렬
    hotspot_list = []
    for file, data in sorted(hotspot_files.items(), key=lambda x: x[1]["lines"], reverse=True)[:10]:
        hotspot_list.append((file, data["lines"], data["freq"]))

    # 커밋 타입 통계
    total = sum(commit_types.values())
    commit_type_stats = []
    for ctype, count in commit_types.most_common():
        pct = round(count / total * 100, 1) if total > 0 else 0
        commit_type_stats.append((ctype, count, pct))

    return {
        "total_commits": len(commits),
        "total_added": total_added,
        "total_deleted": total_deleted,
        "active_days": len(active_days),
        "top_authors": [author for author, _ in authors.most_common(5)],
        "commit_type_stats": commit_type_stats,
        "hotspot_files": hotspot_list,
        "features": features,
        "fixes": fixes,
        "refactors": refactors,
        "performance": performance
    }

def load_daily_logs(devlog_dir, date_from, date_to):
    """Daily DevLog 파일 로드"""
    devlog_path = Path(devlog_dir)
    daily_logs = []

    current = date_from
    while current <= date_to:
        date_str = current.strftime("%Y-%m-%d")
        log_file = devlog_path / f"{date_str}.md"

        if log_file.exists():
            daily_logs.append(date_str)

        current += datetime.timedelta(days=1)

    return daily_logs

def generate_reflection_questions(commits_data):
    """회고 질문 생성"""
    questions = []

    # 기본 질문
    questions.append({
        "q": "이번 주 가장 큰 성과는 무엇이었나요?"
    })

    # 커밋 타입에 따른 질문
    if commits_data["features"]:
        questions.append({
            "q": f"이번 주 {len(commits_data['features'])}개의 신규 기능을 개발했습니다. 그 중 가장 도전적이었던 기능은 무엇이고, 왜 그랬나요?"
        })

    if commits_data["fixes"]:
        questions.append({
            "q": f"{len(commits_data['fixes'])}개의 버그를 수정했습니다. 버그의 근본 원인은 무엇이었고, 재발 방지를 위해 어떤 조치를 취했나요?"
        })

    if commits_data["refactors"]:
        questions.append({
            "q": "리팩토링을 진행한 이유는 무엇이었고, 코드 품질이 실제로 개선되었나요?"
        })

    # Hotspot 관련 질문
    if commits_data["hotspot_files"]:
        top_file = commits_data["hotspot_files"][0][0]
        questions.append({
            "q": f"'{top_file}' 파일이 가장 많이 변경되었습니다. 이 파일이 자주 변경되는 이유는 무엇이고, 구조 개선이 필요할까요?"
        })

    # 다음 주 계획
    questions.append({
        "q": "다음 주에 개선하고 싶은 점은 무엇인가요? (기술적/프로세스적)"
    })

    return questions

def render(context, template_path, out_path):
    """Jinja2 템플릿 렌더링"""
    template_content = Path(template_path).read_text(encoding="utf-8")
    md = Template(template_content).render(**context)

    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    Path(out_path).write_text(md, encoding="utf-8")

def main():
    ap = argparse.ArgumentParser(description="Weekly DevLog Generator")
    ap.add_argument("--date", default=None, help="기준 날짜 (YYYY-MM-DD, 비워두면 이번 주)")
    ap.add_argument("--devlog-dir", default="Documents/DevLog", help="DevLog 디렉토리")
    ap.add_argument("--out", required=True, help="출력 파일 경로")
    ap.add_argument("--template", default=None, help="템플릿 파일 경로")
    args = ap.parse_args()

    # 템플릿 경로 설정
    if not args.template:
        script_dir = Path(__file__).parent
        args.template = script_dir / "weekly_template.md"

    # 주간 범위 계산
    week_range = get_week_range(args.date)
    monday = week_range["monday"]
    sunday = week_range["sunday"]
    week_label = week_range["week_label"]

    print(f"📅 Generating Weekly Report: {week_label}")
    print(f"   Period: {monday} ~ {sunday}")

    # Git 커밋 수집
    since = monday.strftime("%Y-%m-%d 00:00:00")
    until = (sunday + datetime.timedelta(days=1)).strftime("%Y-%m-%d 00:00:00")

    commit_hashes = git_log_range(since, until)
    commits = [parse_commit(h) for h in commit_hashes]
    commits = [c for c in commits if c]  # None 제거

    print(f"   Found {len(commits)} commits")

    # 커밋 분석
    commits_data = analyze_commits(commits)

    # Daily Logs 수집
    daily_log_files = load_daily_logs(args.devlog_dir, monday, sunday)

    # 회고 질문 생성
    reflection_questions = generate_reflection_questions(commits_data)

    # 주간 요약 생성
    summary_parts = []
    if commits_data["features"]:
        summary_parts.append(f"{len(commits_data['features'])}개 신규 기능")
    if commits_data["fixes"]:
        summary_parts.append(f"{len(commits_data['fixes'])}개 버그 수정")
    if commits_data["refactors"]:
        summary_parts.append(f"{len(commits_data['refactors'])}개 리팩토링")

    summary = ", ".join(summary_parts) if summary_parts else "활동 없음"

    # 컨텍스트 구성
    ctx = {
        "week_label": week_label,
        "date_from": monday.isoformat(),
        "date_to": sunday.isoformat(),
        "summary": summary,
        "total_commits": commits_data["total_commits"],
        "total_added": commits_data["total_added"],
        "total_deleted": commits_data["total_deleted"],
        "active_days": commits_data["active_days"],
        "top_authors": commits_data["top_authors"],
        "commit_type_stats": commits_data["commit_type_stats"],
        "hotspot_files": commits_data["hotspot_files"],
        "features": commits_data["features"],
        "fixes": commits_data["fixes"],
        "refactors": commits_data["refactors"],
        "performance": commits_data["performance"],
        "build_stats": None,  # TODO: 빌드 통계 수집
        "test_stats": None,   # TODO: 테스트 통계 수집
        "quality_stats": None,  # TODO: 코드 품질 통계
        "architecture_changes": [],  # TODO: 아키텍처 변화 감지
        "api_changes": None,  # TODO: API 변화 분석
        "issues": [],  # TODO: 이슈 수집
        "risks": [],  # TODO: 위험 요소 분석
        "next_week_priority": [],  # 사용자가 작성
        "tech_debt": [],  # 사용자가 작성
        "reflection_questions": reflection_questions,
        "daily_log_files": daily_log_files,
        "generation_time": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    }

    # 렌더링 및 저장
    render(ctx, args.template, args.out)
    print(f"✅ Weekly DevLog generated: {args.out}")
    print(f"   Summary: {summary}")

if __name__ == "__main__":
    main()
