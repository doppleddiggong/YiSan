# DevLog 자동화 워크플로우 가이드

## 개요

YiSan 프로젝트의 개발 활동을 자동으로 기록하고 성장 피드백을 제공하는 GitHub Actions 워크플로우 시스템입니다.

모든 설정은 `.github/config.yml` 파일에서 중앙 관리됩니다.

## 워크플로우 구조

### 1. Daily DevLog (`devlog-simple.yml`)

**목적**: 매일 개발 활동을 자동으로 기록합니다.

**실행 시점**:
- 매일 KST 오전 9시 (자동)
- 수동 실행 가능

**기능**:
- Git 커밋 내역 수집
- 변경 파일 분석
- 커밋 통계 생성
- **선택적 GPT 피드백**: 수동 실행 시 활성화 가능

**출력**:
- `Documents/DevLog/YYYY-MM-DD.md`
- Discord 알림 (설정 시)

**GPT 피드백 포함 시**:
```markdown
## 🤔 성찰 질문
- 왜 이 방식을 선택했나요?
- 다른 접근 방법은 없었나요?

## 💡 대안 제시
(고려하지 못했을 다른 방법)

## 📚 학습 포인트
(이 작업에서 배울 수 있는 기술 개념)

## ⚠️ 주의 사항
(잠재적 기술 부채, 향후 고려사항)

## 🎯 다음 단계 제안
(개선 작업이나 학습 방향)
```

**사용 방법**:

기본 (GPT 없음):
```bash
# 자동 실행 - 매일 오전 9시
```

GPT 피드백 포함:
```bash
# GitHub Actions > Daily DevLog > Run workflow
# ✅ use_gpt: true
```

---

### 2. Weekly Report (`weekly-report.yml`)

**목적**: 주간 작업 내역을 통합 분석하고 성장 회고를 제공합니다.

**실행 시점**:
- 매주 일요일 KST 오후 11시 (자동, GPT 기본 활성화)
- 수동 실행 가능

**기능**:
- 일주일간 커밋 통계 집계
- Daily DevLog 통합
- 작업 패턴 분석
- **기본 GPT 주간 회고**: 자동 실행 시 포함

**출력**:
- `Documents/DevLog/YYYY-WXX-Summary.md`
- Discord 알림

**GPT 주간 피드백 내용**:
```markdown
## 🏆 이번 주 성과
(구체적 성과 인정)

## 📊 작업 패턴 분석
(강점, 개선 영역)

## 🤔 깊이 있는 회고 질문
- 가장 어려웠던 기술적 도전은?
- 가장 자랑스러운 코드는?
- 다시 한다면 어떻게 다르게 접근할 것인가?

## 💡 놓쳤을 수 있는 관점
(다른 접근 방법, 대안)

## 📚 학습 성장 포인트
(배운 기술 개념, 패턴, 원칙)

## ⚠️ 기술 부채 & 리스크
(향후 문제가 될 수 있는 부분)

## 🎯 다음 주 성장 제안
(구체적이고 실행 가능한 목표)
```

**사용 방법**:

자동 실행 (GPT 포함):
```bash
# 매주 일요일 자동 실행
```

수동 실행 (GPT 제외):
```bash
# GitHub Actions > Weekly DevLog Report > Run workflow
# ⬜ use_gpt: false
```

---

## 설정 방법

### 1. 중앙 설정 파일 (.github/config.yml)

모든 워크플로우 설정은 `.github/config.yml`에서 관리됩니다:

```yaml
# GPT 피드백 설정
gpt:
  daily_enabled: false    # Daily DevLog GPT 피드백 (기본: 꺼짐)
  weekly_enabled: true    # Weekly Report GPT 회고 (기본: 켜짐)

# Discord 알림 설정
discord:
  enabled: true           # Discord 알림 사용 여부

# DevLog 생성 설정
devlog:
  daily_auto: true        # Daily 자동 생성
  weekly_auto: true       # Weekly 자동 생성
  output_dir: "Documents/DevLog"
```

**GPT 기능 켜기/끄기**:
```yaml
# 매일 GPT 피드백 받기 (비용 발생)
gpt:
  daily_enabled: true

# 주간 회고만 GPT 사용 (권장, 비용 효율적)
gpt:
  daily_enabled: false
  weekly_enabled: true

# GPT 완전 비활성화
gpt:
  daily_enabled: false
  weekly_enabled: false
```

### 2. GitHub Secrets 설정

GPT 피드백이나 Discord 알림을 사용하려면 secrets 설정이 필요합니다:

```bash
OPENAI_API_KEY        # GPT 피드백 생성용 (gpt.enabled=true 시 필수)
DISCORD_WEBHOOK_URL   # Discord 알림용 (discord.enabled=true 시 필수)
```

**설정 경로**: Repository Settings > Secrets and variables > Actions

### 3. 우선순위

설정 우선순위는 다음과 같습니다:

1. **수동 실행 입력값** (최우선)
2. **config.yml 설정**
3. **워크플로우 기본값**

예시:
- config.yml에서 `daily_enabled: false`
- 수동 실행 시 `use_gpt: true` 선택
- 결과: GPT 피드백 생성됨 (수동 입력 우선)

---

## 사용 시나리오

### Scenario 1: 빠른 기록만 필요 (GPT 비용 절약)

**config.yml 설정**:
```yaml
gpt:
  daily_enabled: false
  weekly_enabled: false
```

**결과**: 기본 커밋 로그만 생성, API 비용 없음

---

### Scenario 2: 주간 성장 회고 (권장)

**config.yml 설정**:
```yaml
gpt:
  daily_enabled: false
  weekly_enabled: true   # 기본값
```

**결과**: 평일엔 단순 기록, 주말에 GPT 피드백으로 한 주 회고

**비용**: ~$0.20/월

---

### Scenario 3: 매일 즉각 피드백

**config.yml 설정**:
```yaml
gpt:
  daily_enabled: true
  weekly_enabled: true
```

**결과**: 매일 성찰 질문 받고 주말에 통합 회고

**비용**: ~$1.00/월

---

### Scenario 4: 선택적 피드백 (config는 끄고 필요할 때만)

**config.yml 설정**:
```yaml
gpt:
  daily_enabled: false
  weekly_enabled: false
```

**사용법**:
중요한 작업 후 수동으로 실행:
```
GitHub Actions > Daily DevLog > Run workflow
✅ use_gpt: true
```

**결과**: 평소엔 비용 없음, 필요할 때만 피드백

---

## 피드백 활용 가이드

### 1. 일일 피드백 활용

**매일 저녁**:
1. PR에 생성된 DevLog 확인
2. GPT 성찰 질문에 댓글로 답변
3. 제안된 대안 검토
4. 다음날 작업에 반영

**예시**:
```markdown
> 🤔 왜 이 구조를 선택했나요?

답변:
확장성을 고려해 Strategy 패턴을 적용했습니다.
하지만 GPT가 제안한 Decorator 패턴도 고려해볼 만하네요.
내일 리팩토링 시 검토하겠습니다.
```

### 2. 주간 회고 활용

**매주 일요일 밤 / 월요일 아침**:
1. 주간 리포트 읽기
2. 회고 질문에 대한 답변 작성
3. 다음 주 성장 목표 3개 선택
4. 팀 회의에서 공유할 인사이트 정리

**회고 템플릿**:
```markdown
## 이번 주 회고

### 가장 큰 성과
(GPT 피드백 참고하여 작성)

### 어려웠던 점과 극복 과정
(회고 질문에 답변)

### 다음 주 목표
1. (GPT 제안에서 선택)
2.
3.

### 팀 공유 사항
(의미 있는 기술적 인사이트)
```

---

## 문서 구조

```
Documents/
├── DevLog/
│   ├── 2025-01-15.md              # 일일 로그 (기본)
│   ├── 2025-01-16.md              # GPT 피드백 포함 시 자동 병합
│   ├── 2025-W03-Summary.md        # 주간 요약 + GPT 회고
│   └── WORKFLOW_GUIDE.md          # 이 문서
└── SUMMARY.md                      # HonKit 네비게이션
```

---

## FAQ

### Q1. GPT 피드백 설정을 바꾸려면?

`.github/config.yml` 파일을 수정하고 커밋하면 됩니다:

```yaml
gpt:
  daily_enabled: true   # false → true로 변경
```

커밋 후 다음 실행부터 자동 적용됩니다.

### Q2. GPT API 비용은?

- Daily (GPT-4o): ~$0.03 / 회
- Weekly (GPT-4o): ~$0.05 / 회
- 월 예상 비용:
  - Weekly만 (권장): ~$0.20
  - Daily + Weekly: ~$1.00

### Q3. config.yml과 수동 실행 중 뭐가 우선인가요?

**수동 실행이 우선**입니다.

- config: `daily_enabled: false`
- 수동 실행: `use_gpt: true`
- 결과: GPT 피드백 생성됨

config는 자동 실행 시 기본값으로 사용됩니다.

### Q4. Discord 알림을 끄려면?

```yaml
discord:
  enabled: false
```

또는 `DISCORD_WEBHOOK_URL` secret을 삭제하면 됩니다.

### Q5. 기존 DevLog 형식이 변경되나요?

아니요. 기본 DevLog는 동일합니다. GPT 피드백은 파일 하단에 추가됩니다.

### Q6. 피드백이 마음에 들지 않으면?

GPT 피드백은 참고용입니다. 커밋하기 전에 수정하거나 삭제할 수 있습니다.

---

## 성장 중심 개발 문화

이 시스템의 목적은 **단순 기록을 넘어 개발자의 성장**입니다:

1. **성찰**: 오늘 작업의 의미 되새기기
2. **학습**: 기술 개념과 패턴 내재화
3. **개선**: 구체적인 다음 단계 설정
4. **공유**: 팀과 인사이트 나누기

**기억하세요**:
- GPT 피드백은 도구일 뿐, 최종 판단은 개발자 본인이 합니다
- 완벽한 코드보다 지속적인 개선이 중요합니다
- 실수와 시행착오는 성장의 기회입니다

---

**마지막 업데이트**: 2025-11-08
