#!/usr/bin/env python3
"""
GPT 기반 Weekly DevLog Generator
Daily DevLog를 분석하여 기존 형식의 주간 개발 요약을 생성합니다.
"""

import argparse
import subprocess
import os
import datetime
from pathlib import Path
from openai import OpenAI

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
        "week": week_num,
        "date_range": f"{monday.strftime('%Y-%m-%d')} ~ {sunday.strftime('%Y-%m-%d')}"
    }

def load_daily_logs(devlog_dir, week_info):
    """주간 Daily DevLog 파일 전체 내용 수집"""
    devlog_path = Path(devlog_dir)
    daily_logs = []

    current = week_info["monday"]
    while current <= week_info["sunday"]:
        date_str = current.strftime("%Y-%m-%d")
        log_file = devlog_path / f"{date_str}.md"

        if log_file.exists():
            try:
                content = log_file.read_text(encoding="utf-8")
                daily_logs.append({
                    "date": date_str,
                    "filename": f"{date_str}.md",
                    "content": content
                })
            except Exception as e:
                print(f"⚠️ {date_str}.md 읽기 실패: {e}")

        current += datetime.timedelta(days=1)

    return daily_logs

def get_week_commits(week_info):
    """주간 커밋 통계"""
    since = week_info["monday"].strftime("%Y-%m-%d 00:00:00")
    until = (week_info["sunday"] + datetime.timedelta(days=1)).strftime("%Y-%m-%d 00:00:00")

    # 커밋 수집
    commits = []
    hashes = sh(f'git log --since="{since}" --until="{until}" --pretty=format:"%H"')

    if not hashes:
        return {
            "count": 0,
            "added": 0,
            "deleted": 0,
            "details": []
        }

    for h in hashes.splitlines():
        subject = sh(f'git show --format="%s" --no-patch {h}')
        author = sh(f'git show --format="%an" --no-patch {h}')
        date = sh(f'git show --format="%ai" --no-patch {h}')

        commits.append({
            "hash": h[:7],
            "subject": subject,
            "author": author,
            "date": date[:10]
        })

    # 변경 라인 통계
    stats = sh(f'git log --since="{since}" --until="{until}" --numstat --pretty=format:""')
    added = deleted = 0
    for line in stats.splitlines():
        if not line.strip():
            continue
        parts = line.split()
        if len(parts) >= 2:
            try:
                added += int(parts[0]) if parts[0] != '-' else 0
                deleted += int(parts[1]) if parts[1] != '-' else 0
            except:
                pass

    return {
        "count": len(commits),
        "added": added,
        "deleted": deleted,
        "details": commits[:20]  # 최대 20개
    }

def generate_weekly_summary_with_gpt(daily_logs, commit_stats, week_info, api_key):
    """GPT-4로 기존 형식의 주간 개발 요약 생성"""

    client = OpenAI(api_key=api_key)

    # Daily Logs 요약 (파일명과 주요 내용)
    daily_summary = ""
    for log in daily_logs:
        # 각 Daily Log의 첫 100줄 정도만 (핵심 변경 사항 위주)
        lines = log["content"].split("\n")
        key_sections = "\n".join(lines[:100])
        daily_summary += f"\n\n### {log['date']} ({log['filename']})\n{key_sections}\n"

    # 커밋 요약
    commit_summary = f"총 커밋: {commit_stats['count']}개\n"
    commit_summary += f"변경 라인: +{commit_stats['added']} / -{commit_stats['deleted']}\n\n"
    if commit_stats['details']:
        commit_summary += "주요 커밋:\n"
        for c in commit_stats['details'][:10]:
            commit_summary += f"- {c['date']} [{c['author']}] {c['subject']}\n"

    system_prompt = """당신은 경험 많은 프로젝트 리더이자 기술 문서 작성 전문가입니다.
개발팀의 주간 작업 내역을 분석하여 구조화된 주간 개발 요약 보고서를 작성합니다.

**중요**: 반드시 다음 형식을 정확히 따라주세요:

# 주간 개발 요약 (날짜 범위)

## 주간 개요 / Weekly Overview
- **핵심 주제 1**: 한 주 동안의 주요 작업을 3-4개의 bullet point로 요약
- **핵심 주제 2**: 각 bullet point는 해당 작업의 목적과 성과를 설명
- **핵심 주제 3**: 기술적 맥락을 포함하여 작성

## 핵심 성과 / Key Achievements
1. **성과 1 제목** — 성과에 대한 상세 설명. 어떤 컴포넌트/시스템을 개선했는지, 왜 중요한지 설명.
2. **성과 2 제목** — 구체적인 기술 스택, 클래스명, 시스템명을 언급하며 설명.
3. **성과 3 제목** — (계속...)

## 리스크 및 이슈 / Risks & Issues
- **이슈 제목**: 구체적인 이슈 설명과 왜 위험한지, 향후 영향 분석
- **리스크 제목**: (계속...)

## 다음 주 우선순위 / Next Week Priorities
1. **우선순위 1** — 왜 이 작업이 필요한지, 기대 효과
2. **우선순위 2** — (계속...)

## 역할별 리뷰 / Role-based Review
- **클라이언트팀장 관점**: UI/UX, 사용자 경험 측면의 분석
- **네트워크 리드 관점**: 네트워크, 멀티플레이, 안정성 측면의 분석
- **디자인/콘텐츠 관점**: 콘텐츠, 에셋, 비주얼 측면의 분석

## 구성원 역량 평가 / Individual Competency Review
- **개발자명**: 이번 주 기여도와 기술 성장, 강점 및 개선 제안
- (팀원 수 만큼...)

## 메트릭 / Metrics
- **주간 변경량**: +X / -Y (특이사항 설명)
- **활동 일수**: X/7일 — 코멘트
- **주요 테마**: 이번 주 핵심 주제 요약

**작성 원칙**:
1. 기술적으로 구체적이되 간결하게
2. Daily Log의 내용을 종합하여 큰 그림 제시
3. 한국어와 영어 섹션명 병기
4. 역할별, 개인별 관점 포함
5. 정량적 메트릭 포함
"""

    user_prompt = f"""다음은 {week_info['date_range']} 주간의 작업 내역입니다:

## Daily DevLog 내용
{daily_summary[:15000]}

## 커밋 통계
{commit_summary}

---

위 내용을 종합하여 기존 형식의 주간 개발 요약을 작성해주세요.
반드시 지정된 형식과 섹션 구조를 따라주세요.
"""

    try:
        response = client.chat.completions.create(
            model="gpt-4o",
            messages=[
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ],
            temperature=0.7,
            max_tokens=4000
        )

        summary = response.choices[0].message.content
        return summary

    except Exception as e:
        print(f"❌ GPT API 호출 실패: {e}")
        return None

def main():
    ap = argparse.ArgumentParser(description="GPT 기반 Weekly DevLog Generator")
    ap.add_argument("--date", default=None, help="기준 날짜 (YYYY-MM-DD, 비워두면 이번 주)")
    ap.add_argument("--devlog-dir", required=True, help="Daily DevLog 디렉토리")
    ap.add_argument("--out", required=True, help="출력 파일 경로")
    args = ap.parse_args()

    # API 키 확인
    api_key = os.getenv("OPENAI_API_KEY")
    if not api_key:
        print("❌ OPENAI_API_KEY 환경 변수가 설정되지 않았습니다.")
        print("   GPT 없이 기본 통계만 생성하려면 generate_weekly.py를 사용하세요.")
        return 1

    # 주간 범위 계산
    week_info = get_week_range(args.date)
    print(f"📅 주차: {week_info['week_label']}")
    print(f"   기간: {week_info['date_range']}")

    # Daily Logs 수집
    print(f"📖 Daily DevLog 수집 중...")
    daily_logs = load_daily_logs(args.devlog_dir, week_info)
    print(f"   수집된 일일 로그: {len(daily_logs)}개")

    if not daily_logs:
        print("⚠️ Daily DevLog가 없습니다. 주간 요약을 생성할 수 없습니다.")
        return 1

    # 커밋 통계 수집
    print(f"📊 커밋 통계 수집 중...")
    commit_stats = get_week_commits(week_info)
    print(f"   총 커밋: {commit_stats['count']}개")

    # GPT로 주간 요약 생성
    print(f"🤖 GPT-4로 주간 개발 요약 생성 중...")
    summary = generate_weekly_summary_with_gpt(daily_logs, commit_stats, week_info, api_key)

    if not summary:
        print("❌ 주간 요약 생성 실패")
        return 1

    # 파일 저장
    output_path = Path(args.out)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(summary, encoding="utf-8")

    print(f"✅ 주간 개발 요약 생성 완료: {args.out}")
    print(f"   파일명: {week_info['week_label']}-Summary.md")

    return 0

if __name__ == "__main__":
    exit(main())
