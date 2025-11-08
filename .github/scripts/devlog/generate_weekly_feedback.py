#!/usr/bin/env python3
"""
GPT Weekly Feedback Generator
주간 작업을 분석하여 성장 중심의 회고 피드백을 생성합니다.
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

def get_week_commits(week_label):
    """주차에 해당하는 커밋 수집"""
    # week_label 형식: "2025-W01"
    parts = week_label.split("-W")
    if len(parts) != 2:
        return []

    year = int(parts[0])
    week = int(parts[1])

    # 해당 주의 월요일과 일요일 계산
    jan1 = datetime.date(year, 1, 1)
    week_start = jan1 + datetime.timedelta(weeks=week-1, days=-jan1.weekday())
    week_end = week_start + datetime.timedelta(days=6)

    since = week_start.strftime("%Y-%m-%d 00:00:00")
    until = (week_end + datetime.timedelta(days=1)).strftime("%Y-%m-%d 00:00:00")

    # 커밋 수집
    commits = []
    hashes = sh(f'git log --since="{since}" --until="{until}" --pretty=format:"%H"')

    if not hashes:
        return commits

    for h in hashes.splitlines():
        subject = sh(f'git show --format="%s" --no-patch {h}')
        body = sh(f'git show --format="%b" --no-patch {h}')
        author = sh(f'git show --format="%an" --no-patch {h}')
        date = sh(f'git show --format="%ai" --no-patch {h}')

        diff_stat = sh(f'git show --stat {h}')

        commits.append({
            "hash": h[:7],
            "subject": subject,
            "body": body,
            "author": author,
            "date": date[:10],
            "diff_stat": diff_stat[:500]
        })

    return commits

def load_daily_logs(devlog_dir, week_label):
    """주간 Daily DevLog 파일 내용 수집"""
    # week_label에서 주차 계산
    parts = week_label.split("-W")
    if len(parts) != 2:
        return []

    year = int(parts[0])
    week = int(parts[1])

    jan1 = datetime.date(year, 1, 1)
    week_start = jan1 + datetime.timedelta(weeks=week-1, days=-jan1.weekday())
    week_end = week_start + datetime.timedelta(days=6)

    devlog_path = Path(devlog_dir)
    daily_contents = []

    current = week_start
    while current <= week_end:
        date_str = current.strftime("%Y-%m-%d")
        log_file = devlog_path / f"{date_str}.md"

        if log_file.exists():
            try:
                content = log_file.read_text(encoding="utf-8")
                daily_contents.append({
                    "date": date_str,
                    "content": content[:2000]  # 각 파일 최대 2000자
                })
            except:
                pass

        current += datetime.timedelta(days=1)

    return daily_contents

def generate_weekly_feedback(weekly_report, commits, daily_logs, api_key):
    """GPT-4로 주간 성장 피드백 생성"""

    client = OpenAI(api_key=api_key)

    # 커밋 요약
    commit_summary = "\n\n".join([
        f"**{c['date']} - {c['hash']}**: {c['subject']}\n{c['diff_stat'][:200]}"
        for c in commits[:10]  # 최대 10개
    ])

    # Daily Log 요약
    daily_summary = "\n\n".join([
        f"**{d['date']}**:\n{d['content'][:800]}\n..."
        for d in daily_logs[:5]  # 최대 5일
    ])

    system_prompt = """당신은 경험 많은 소프트웨어 엔지니어링 멘토입니다.
개발자의 주간 작업 내역을 분석하여 성장 중심의 회고와 피드백을 제공합니다.

피드백 철학:
1. **성과 인정**: 이번 주의 긍정적인 성과를 구체적으로 인정
2. **패턴 발견**: 작업 패턴에서 보이는 강점과 개선점 도출
3. **기술 성장**: 새롭게 학습한 기술이나 개념 강조
4. **다음 단계**: 실질적이고 구체적인 다음 주 성장 방향 제시

톤:
- 격려하되 솔직하게
- 구체적 사례 기반
- 실행 가능한 조언
"""

    user_prompt = f"""다음은 개발자의 이번 주 작업 내역입니다:

## 주간 리포트
{weekly_report[:3000]}

## 상세 커밋 내역
{commit_summary}

## 일일 작업 로그 (샘플)
{daily_summary}

---

위 내용을 분석하여 다음 형식으로 주간 회고 피드백을 작성해주세요:

## 🏆 이번 주 성과

(구체적인 성과 3-5가지를 인정하고 칭찬)

## 📊 작업 패턴 분석

(이번 주 작업에서 보이는 패턴, 강점, 개선 필요 영역)

## 🤔 깊이 있는 회고 질문

(개발자가 스스로 답해야 할 성찰 질문 5-7개)

예시:
- 이번 주 가장 어려웠던 기술적 도전은 무엇이었고, 어떻게 극복했나요?
- 작성한 코드 중 가장 자랑스러운 부분은 무엇이고, 그 이유는?
- 같은 문제를 다시 만난다면 어떻게 다르게 접근하시겠습니까?

## 💡 놓쳤을 수 있는 관점

(다른 접근 방법, 고려하지 못한 기술적 대안, 베스트 프랙티스)

## 📚 학습 성장 포인트

(이번 주 작업에서 배운 기술 개념, 패턴, 원칙을 정리)

## ⚠️ 기술 부채 & 리스크

(향후 문제가 될 수 있는 부분, 리팩토링 필요 영역)

## 🎯 다음 주 성장 제안

(구체적이고 실행 가능한 다음 주 목표 3-5개)

---

각 섹션을 구체적인 사례와 함께 작성해주세요.
개발자가 실제로 행동에 옮길 수 있는 피드백을 제공하는 것이 목표입니다.
"""

    try:
        response = client.chat.completions.create(
            model="gpt-4o",
            messages=[
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ],
            temperature=0.7,
            max_tokens=3000
        )

        feedback = response.choices[0].message.content
        return feedback

    except Exception as e:
        print(f"❌ GPT API 호출 실패: {e}")
        return None

def main():
    ap = argparse.ArgumentParser(description="Generate GPT-4 weekly feedback")
    ap.add_argument("--weekly-file", required=True, help="주간 리포트 파일 경로")
    ap.add_argument("--devlog-dir", required=True, help="DevLog 디렉토리")
    ap.add_argument("--week-label", required=True, help="주차 라벨 (예: 2025-W01)")
    ap.add_argument("--output", required=True, help="출력 파일 경로")
    args = ap.parse_args()

    # API 키 확인
    api_key = os.getenv("OPENAI_API_KEY")
    if not api_key:
        print("❌ OPENAI_API_KEY 환경 변수가 설정되지 않았습니다.")
        return

    # 주간 리포트 읽기
    weekly_path = Path(args.weekly_file)
    if not weekly_path.exists():
        print(f"❌ 주간 리포트 파일을 찾을 수 없습니다: {args.weekly_file}")
        return

    weekly_content = weekly_path.read_text(encoding="utf-8")

    # 주간 커밋 수집
    print(f"📊 주차 {args.week_label} 커밋 정보 수집 중...")
    commits = get_week_commits(args.week_label)
    print(f"   수집된 커밋: {len(commits)}개")

    # Daily Logs 수집
    print(f"📖 Daily DevLog 수집 중...")
    daily_logs = load_daily_logs(args.devlog_dir, args.week_label)
    print(f"   수집된 일일 로그: {len(daily_logs)}개")

    if not commits and not daily_logs:
        print("⚠️ 분석할 데이터가 없습니다. 피드백을 생성하지 않습니다.")
        return

    # GPT 피드백 생성
    print("🤖 GPT-4로 주간 회고 피드백 생성 중...")
    feedback = generate_weekly_feedback(weekly_content, commits, daily_logs, api_key)

    if not feedback:
        print("❌ 피드백 생성 실패")
        return

    # 피드백 파일 저장
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    final_content = f"""
---

# 🎓 주간 성장 회고 (GPT-4 Weekly Feedback)

{feedback}

---

## 📝 회고 작성 가이드

위 피드백을 참고하여 스스로 회고를 작성해보세요:

1. **회고 질문에 답하기**: 각 질문에 대해 솔직하게 답변을 작성하세요
2. **성장 포인트 내재화**: 학습한 내용을 자신의 언어로 정리하세요
3. **다음 주 계획**: 제안된 목표 중 실제로 실행할 것을 선택하고 구체화하세요
4. **팀 공유**: 의미 있는 인사이트는 팀과 공유하세요

---

*이 피드백은 OpenAI GPT-4를 통해 자동 생성되었습니다.*
*피드백을 참고하되, 최종 판단과 회고는 본인이 직접 작성하시기 바랍니다.*
"""

    output_path.write_text(final_content, encoding="utf-8")
    print(f"✅ GPT 주간 피드백 생성 완료: {args.output}")

if __name__ == "__main__":
    main()
