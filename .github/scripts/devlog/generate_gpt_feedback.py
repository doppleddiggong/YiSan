#!/usr/bin/env python3
"""
GPT Feedback Generator for DevLog
개발자 성장을 위한 피드백을 GPT-4를 통해 생성합니다.
"""

import argparse
import subprocess
import os
from pathlib import Path
from openai import OpenAI

def sh(cmd):
    """셸 명령 실행"""
    try:
        return subprocess.check_output(cmd, shell=True, text=True, errors="ignore").strip()
    except subprocess.CalledProcessError:
        return ""

def get_commit_details(since):
    """최근 커밋의 상세 정보 수집"""
    commits = []

    # 커밋 해시 목록 가져오기
    hashes = sh(f'git log --since="{since}" --pretty=format:"%H"')
    if not hashes:
        return commits

    for h in hashes.splitlines():
        # 커밋 메시지
        subject = sh(f'git show --format="%s" --no-patch {h}')
        body = sh(f'git show --format="%b" --no-patch {h}')
        author = sh(f'git show --format="%an" --no-patch {h}')

        # 변경된 파일 목록
        files = sh(f'git show --name-only --format="" {h}')

        # 변경 내용 diff (간략하게)
        diff = sh(f'git show --stat {h}')

        commits.append({
            "hash": h[:7],
            "subject": subject,
            "body": body,
            "author": author,
            "files": files.splitlines() if files else [],
            "diff_stat": diff
        })

    return commits

def generate_gpt_feedback(devlog_content, commit_details, api_key):
    """GPT-4를 사용하여 개발자 성장 피드백 생성"""

    client = OpenAI(api_key=api_key)

    # 커밋 요약 생성
    commit_summary = "\n\n".join([
        f"**커밋 {c['hash']}**: {c['subject']}\n파일: {', '.join(c['files'][:5])}\n{c['diff_stat'][:300]}"
        for c in commit_details[:5]  # 최대 5개 커밋만
    ])

    system_prompt = """당신은 시니어 소프트웨어 엔지니어이자 멘토입니다.
개발자의 일일 개발 로그를 분석하여 성장을 돕는 건설적인 피드백을 제공합니다.

피드백 원칙:
1. **성찰 질문**: 개발자가 스스로 생각하게 만드는 질문 (Why? How?)
2. **대안 제시**: 고려하지 못했을 다른 접근 방법 제안
3. **학습 포인트**: 이 작업에서 배울 수 있는 기술적 개념이나 패턴
4. **주의 사항**: 기술 부채, 잠재적 버그, 유지보수성 이슈

피드백 톤:
- 비판적이되 격려적으로
- 구체적이고 실행 가능하게
- 개발자의 의도를 존중하되 개선점 명확히 지적
"""

    user_prompt = f"""다음은 개발자의 오늘 작업 내용입니다:

## DevLog
{devlog_content[:3000]}

## 상세 커밋 정보
{commit_summary}

---

위 작업 내용을 분석하여 다음 형식으로 피드백을 작성해주세요:

## 🤔 성찰 질문
(개발자가 고민해볼 만한 3-5가지 질문)

## 💡 대안 제시
(다르게 접근할 수 있었던 방법이나 개선 아이디어)

## 📚 학습 포인트
(이 작업에서 배울 수 있는 기술 개념, 디자인 패턴, 베스트 프랙티스)

## ⚠️ 주의 사항
(잠재적 문제점, 기술 부채, 향후 고려해야 할 사항)

## 🎯 다음 단계 제안
(이 작업을 기반으로 시도해볼 만한 개선 작업이나 학습 방향)

---

각 섹션은 3-5개의 bullet point로 구체적이고 실용적으로 작성해주세요.
코드 예시가 필요하면 간단히 포함하되, 너무 길지 않게 해주세요.
"""

    try:
        response = client.chat.completions.create(
            model="gpt-4o",
            messages=[
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ],
            temperature=0.7,
            max_tokens=2000
        )

        feedback = response.choices[0].message.content
        return feedback

    except Exception as e:
        print(f"❌ GPT API 호출 실패: {e}")
        return None

def main():
    ap = argparse.ArgumentParser(description="Generate GPT-4 feedback for DevLog")
    ap.add_argument("--devlog-file", required=True, help="DevLog 파일 경로")
    ap.add_argument("--since", default="24 hours", help="Git log 시작 시간")
    ap.add_argument("--output", required=True, help="출력 파일 경로")
    args = ap.parse_args()

    # API 키 확인
    api_key = os.getenv("OPENAI_API_KEY")
    if not api_key:
        print("❌ OPENAI_API_KEY 환경 변수가 설정되지 않았습니다.")
        return

    # DevLog 읽기
    devlog_path = Path(args.devlog_file)
    if not devlog_path.exists():
        print(f"❌ DevLog 파일을 찾을 수 없습니다: {args.devlog_file}")
        return

    devlog_content = devlog_path.read_text(encoding="utf-8")

    # 커밋 상세 정보 수집
    print("📊 커밋 정보 수집 중...")
    commit_details = get_commit_details(args.since)

    if not commit_details:
        print("⚠️ 분석할 커밋이 없습니다. 피드백을 생성하지 않습니다.")
        return

    print(f"✅ {len(commit_details)}개 커밋 분석 준비 완료")

    # GPT 피드백 생성
    print("🤖 GPT-4로 피드백 생성 중...")
    feedback = generate_gpt_feedback(devlog_content, commit_details, api_key)

    if not feedback:
        print("❌ 피드백 생성 실패")
        return

    # 피드백 파일 저장
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # 피드백 앞에 구분선 추가
    final_content = f"""
---

# 🎓 개발자 성장 피드백 (GPT-4 Analysis)

{feedback}

---

*이 피드백은 OpenAI GPT-4를 통해 자동 생성되었습니다. 참고용으로 활용하시고, 최종 판단은 개발자 본인이 내리시기 바랍니다.*
"""

    output_path.write_text(final_content, encoding="utf-8")
    print(f"✅ GPT 피드백 생성 완료: {args.output}")

if __name__ == "__main__":
    main()
