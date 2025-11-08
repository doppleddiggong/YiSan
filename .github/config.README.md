# DevLog Automation Config Guide

`.github/config.yml` 설정 파일 사용 가이드입니다.

## 빠른 시작

### 1. GPT 피드백 켜기/끄기

```yaml
gpt:
  daily_enabled: false   # Daily DevLog GPT 피드백
  weekly_enabled: true   # Weekly Report GPT 회고
```

### 2. Discord 알림 설정

```yaml
discord:
  enabled: true   # Discord 알림 사용
```

## 설정 항목

### GPT 설정 (gpt)

| 항목 | 기본값 | 설명 |
|------|--------|------|
| `daily_enabled` | `false` | 매일 GPT 피드백 생성 여부 |
| `weekly_enabled` | `true` | 주간 GPT 회고 생성 여부 |

**비용 고려**:
- `daily_enabled: true` → 월 ~$0.90 추가 비용
- `weekly_enabled: true` → 월 ~$0.20 비용

**권장 설정**:
```yaml
gpt:
  daily_enabled: false    # 비용 절약
  weekly_enabled: true    # 주간 회고만
```

### Discord 설정 (discord)

| 항목 | 기본값 | 설명 |
|------|--------|------|
| `enabled` | `true` | Discord Webhook 알림 사용 여부 |

**주의**: `DISCORD_WEBHOOK_URL` secret이 설정되어 있어야 합니다.

### DevLog 설정 (devlog)

| 항목 | 기본값 | 설명 |
|------|--------|------|
| `daily_auto` | `true` | Daily DevLog 자동 생성 |
| `weekly_auto` | `true` | Weekly Report 자동 생성 |
| `output_dir` | `"Documents/DevLog"` | 출력 디렉토리 |

### 성능 설정 (performance)

| 항목 | 기본값 | 설명 |
|------|--------|------|
| `git_log_hours` | `24` | Git log 조회 기간 (시간) |
| `weekly_max_daily_logs` | `7` | 주간 리포트 분석 대상 일수 |

### 메시지 설정 (messages)

커밋 메시지와 알림 메시지를 커스터마이징할 수 있습니다.

```yaml
messages:
  daily_commit_prefix: "📝 DevLog:"
  weekly_commit_prefix: "📊 Weekly:"
  no_commits_message: "No commits today"
```

## 실전 예제

### 예제 1: 비용 최소화

```yaml
gpt:
  daily_enabled: false
  weekly_enabled: false
discord:
  enabled: false
```

**결과**: GPT/Discord 완전 비활성화, 기본 로그만 생성

---

### 예제 2: 비용 효율적 (권장)

```yaml
gpt:
  daily_enabled: false
  weekly_enabled: true
discord:
  enabled: true
```

**결과**: 주간 회고만 GPT 사용, Discord 알림 받음

**비용**: ~$0.20/월

---

### 예제 3: 매일 피드백

```yaml
gpt:
  daily_enabled: true
  weekly_enabled: true
discord:
  enabled: true
```

**결과**: 매일 + 주간 GPT 피드백, Discord 알림

**비용**: ~$1.00/월

---

## 설정 변경 방법

### 1. 파일 수정

```bash
# config.yml 수정
vim .github/config.yml

# 또는 GitHub UI에서 수정
# .github/config.yml 파일 열기 → Edit 버튼
```

### 2. 커밋 & 푸시

```bash
git add .github/config.yml
git commit -m "chore: update devlog config"
git push
```

### 3. 즉시 적용

다음 워크플로우 실행부터 새 설정이 적용됩니다.

**테스트 방법**:
```bash
# GitHub Actions > Daily DevLog > Run workflow
# 수동 실행으로 바로 테스트 가능
```

---

## 우선순위

설정 적용 우선순위:

1. **수동 실행 입력값** (최우선)
2. **config.yml 설정**
3. **워크플로우 기본값** (최후)

**예시**:
- `config.yml`: `daily_enabled: false`
- 수동 실행: `use_gpt: true`
- **결과**: GPT 사용됨 (수동 입력 우선)

---

## 문제 해결

### GPT가 작동하지 않아요

1. `OPENAI_API_KEY` secret 확인
2. `config.yml`의 `gpt.enabled` 확인
3. API 할당량 확인

### Discord 알림이 안 와요

1. `DISCORD_WEBHOOK_URL` secret 확인
2. `config.yml`의 `discord.enabled` 확인
3. Webhook URL 유효성 확인

### 설정을 바꿨는데 적용 안 돼요

1. config.yml을 커밋했는지 확인
2. main 브랜치에 푸시했는지 확인
3. 다음 자동 실행까지 대기 또는 수동 실행

---

**더 자세한 내용**: [DevLog 워크플로우 가이드](../Documents/DevLog/WORKFLOW_GUIDE.md)
