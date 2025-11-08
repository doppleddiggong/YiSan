# Discord Webhook 설정 가이드

DevLog 자동 알림을 Discord로 받기 위한 설정 방법입니다.

## 1. Discord Webhook URL 생성

### 1.1 Discord 서버에서 Webhook 생성

1. Discord 서버 설정 → **통합(Integrations)** 메뉴로 이동
2. **웹후크(Webhooks)** 선택
3. **새 웹후크(New Webhook)** 버튼 클릭
4. Webhook 설정:
   - **이름**: DevLog Bot (원하는 이름)
   - **채널**: DevLog를 받을 채널 선택
   - **아이콘**: (선택사항) 봇 아이콘 설정

5. **웹후크 URL 복사** 버튼 클릭하여 URL 복사
   - 형식: `https://discord.com/api/webhooks/[WEBHOOK_ID]/[WEBHOOK_TOKEN]`

## 2. GitHub Repository에 Secret 등록

### 2.1 Repository Settings로 이동

1. GitHub Repository 페이지에서 **Settings** 탭 클릭
2. 왼쪽 메뉴에서 **Secrets and variables** → **Actions** 선택

### 2.2 New Repository Secret 생성

1. **New repository secret** 버튼 클릭
2. Secret 정보 입력:
   - **Name**: `DISCORD_WEBHOOK_URL`
   - **Secret**: 복사한 Discord Webhook URL 붙여넣기
3. **Add secret** 버튼 클릭

## 3. 테스트

### 3.1 수동 워크플로우 실행

1. GitHub Repository의 **Actions** 탭으로 이동
2. **Daily DevLog (Simple)** 워크플로우 선택
3. **Run workflow** 버튼 클릭
4. Discord 채널에서 메시지가 도착하는지 확인

## 4. Discord 메시지 예시

### Daily DevLog 알림

```
📅 Daily DevLog — 2025-01-08

오늘의 개발 활동이 기록되었습니다.

🎯 주요 변경 사항
✨ feat: 새로운 로딩 시스템 구현
🐛 fix: 네트워크 동기화 버그 수정
♻️ refactor: 캐릭터 시스템 리팩토링

📈 통계
📊 커밋: 5개
➕ 추가: 234 라인
➖ 삭제: 89 라인

💡 DevLog를 확인하고 피드백을 남겨주세요!
```

### Weekly Report 알림

```
📊 Weekly Report — 2025-W02

5개 신규 기능, 3개 버그 수정, 2개 리팩토링

📈 주간 통계
📊 총 커밋: 23개
📅 활동 일수: 5/7일
✨ 신규 기능: 5개
🐛 버그 수정: 3개

💭 회고 시간!
Weekly Report에 포함된 회고 질문에 답변해주세요.
팀원들과 함께 이번 주를 돌아보고 다음 주를 계획해봅시다! 🚀

✍️ 회고 질문에 답변을 작성하고 함께 성장하세요!
```

## 5. 피드백 작성 방법

### 5.1 Weekly Report 회고 질문 답변

1. Discord 메시지의 링크를 클릭하여 HonKit 문서 페이지로 이동
2. GitHub에서 해당 Weekly Report 파일 열기:
   - `Documents/DevLog/YYYY-WXX-Summary.md`
3. **Edit this file** 버튼 클릭 (또는 직접 Pull Request 생성)
4. **회고 및 피드백** 섹션의 질문에 답변 작성
5. Commit 및 Push (또는 PR 생성)

### 5.2 Daily DevLog 피드백

Discord 메시지에 스레드로 간단한 피드백을 남길 수 있습니다:
- 👍 잘했어요!
- 💡 아이디어나 제안
- ❓ 질문이나 토론 주제

## 6. 문제 해결

### Webhook URL이 작동하지 않는 경우

1. Discord Webhook URL이 올바른지 확인
2. GitHub Secret이 정확히 등록되었는지 확인
3. Webhook이 삭제되지 않았는지 Discord에서 확인

### 메시지가 전송되지 않는 경우

1. GitHub Actions 로그에서 에러 메시지 확인
2. Discord 채널의 권한 설정 확인 (Webhook이 메시지를 보낼 수 있는지)
3. Rate Limit에 걸리지 않았는지 확인 (Discord는 분당 30개 메시지 제한)

## 7. 고급 설정

### 7.1 여러 채널에 알림 보내기

Daily와 Weekly를 다른 채널로 보내고 싶은 경우:

1. 각 채널별로 Webhook 생성
2. GitHub Secrets에 추가:
   - `DISCORD_WEBHOOK_DAILY`
   - `DISCORD_WEBHOOK_WEEKLY`
3. Workflow 파일 수정하여 각각 다른 Secret 사용

### 7.2 멘션 추가

특정 역할이나 사용자를 멘션하려면:

`send_discord.py`의 `payload`에 `content` 추가:

```python
payload = {
    "content": "<@&ROLE_ID>",  # 역할 멘션
    # 또는
    "content": "<@USER_ID>",   # 사용자 멘션
    "username": username,
    "embeds": [embed]
}
```

---

**설정 완료 후 자동으로 DevLog 알림을 받게 됩니다!** 🎉
