---
skill: honkit-devlog
description: HonKit 기반 DevLog 자동화 시스템을 프로젝트에 설정합니다
version: 1.0.0
author: DoppledDiggong Team
---

# HonKit DevLog Automation Skill

이 스킬은 프로젝트에 HonKit 기반의 자동화된 개발 문서 시스템을 설정합니다.

## 제공 기능

### 1. HonKit 문서화 시스템
- **Documents 폴더 구조** 자동 생성
- **book.json** 설정 파일 생성
- **SUMMARY.md** 자동 생성 스크립트
- **GitHub Actions** 워크플로우 (자동 빌드 및 배포)

### 2. Daily DevLog 자동 생성
- Git 커밋 통계 수집
- Conventional Commits 기반 분류
- Hotspot 파일 분석
- Mermaid 다이어그램 자동 생성
- 매일 자동 실행 (GitHub Actions)

### 3. Weekly Report 자동 집계
- 7일간의 커밋 분석
- 주간 통계 및 트렌드
- 회고 질문 자동 생성
- 매주 일요일 자동 실행

### 4. System Review (주간/월간)
- 아키텍처 변화 분석
- Hotspot 및 리스크 평가
- 코드 건강도 메트릭
- 성능 및 안정성 추적

### 5. Discord Webhook 연동
- Daily/Weekly 알림 자동 전송
- 피드백 유도 시스템
- 회고 질문 공유

## 설정 방법

### 필수 사항
- GitHub Repository
- Python 3.11+ (GitHub Actions에서 자동 설치)
- (선택) Discord Webhook URL

### 1단계: 디렉토리 구조 생성

```bash
mkdir -p Documents/{DevLog,Planning,SystemReview}
mkdir -p .github/{workflows,scripts/devlog}
```

### 2단계: HonKit 설정 파일 생성

**Documents/book.json**:
```json
{
  "title": "[프로젝트명] Development Documentation",
  "description": "[프로젝트 설명]",
  "author": "[작성자명]",
  "language": "ko",
  "gitbook": "3.2.3",
  "plugins": [
    "github",
    "mermaid-gb3",
    "collapsible-menu",
    "back-to-top-button",
    "search-plus",
    "expandable-chapters-small",
    "-lunr",
    "-search"
  ],
  "pluginsConfig": {
    "github": {
      "url": "https://github.com/[USERNAME]/[REPO]"
    }
  }
}
```

### 3단계: Python 스크립트 복사

다음 파일들을 `.github/scripts/devlog/`에 복사:

1. **generate_daily.py** - Daily DevLog 생성
2. **daily_template.md** - Daily 템플릿
3. **generate_weekly.py** - Weekly Report 생성
4. **weekly_template.md** - Weekly 템플릿
5. **generate_system_review.py** - System Review 생성
6. **system_review_template.md** - System Review 템플릿
7. **update_summary.py** - SUMMARY.md 자동 업데이트
8. **send_discord.py** - Discord 알림 전송

### 4단계: GitHub Actions 워크플로우 설정

`.github/workflows/`에 다음 파일들을 생성:

1. **honkit.yml** - HonKit 빌드 및 배포
2. **devlog-simple.yml** - Daily DevLog 자동 생성
3. **weekly-report.yml** - Weekly Report 자동 생성
4. **system-review.yml** - System Review 자동 생성

### 5단계: GitHub Pages 활성화

1. Repository Settings → Pages
2. Source: Deploy from a branch
3. Branch: `gh-pages` / `/ (root)` 선택
4. Save

### 6단계: Discord Webhook 설정 (선택)

1. Discord 서버에서 Webhook URL 생성
2. GitHub Repository Settings → Secrets and variables → Actions
3. New repository secret:
   - Name: `DISCORD_WEBHOOK_URL`
   - Secret: Discord Webhook URL

## 사용 방법

### 자동 실행 스케줄

| 워크플로우 | 스케줄 | 설명 |
|-----------|--------|------|
| Daily DevLog | 매일 오전 9시 (KST) | 전날 커밋 분석 및 로그 생성 |
| Weekly Report | 매주 일요일 오후 11시 (KST) | 주간 집계 및 회고 질문 |
| System Review | 매월 1일 오후 11시 (KST) | 월간 시스템 리뷰 |
| HonKit Build | DevLog/Planning 업데이트 시 | 문서 빌드 및 배포 |

### 수동 실행

GitHub Actions 탭에서 원하는 워크플로우를 선택하고 "Run workflow" 버튼 클릭

### 로컬 테스트

```bash
# Daily DevLog 생성
python .github/scripts/devlog/generate_daily.py \
  --since "24 hours" \
  --branch "main" \
  --base "origin/main" \
  --out "Documents/DevLog/2025-01-08.md"

# Weekly Report 생성
python .github/scripts/devlog/generate_weekly.py \
  --devlog-dir "Documents/DevLog" \
  --out "Documents/DevLog/2025-W02-Summary.md"

# System Review 생성
python .github/scripts/devlog/generate_system_review.py \
  --type "monthly" \
  --devlog-dir "Documents/DevLog" \
  --out-dir "Documents/SystemReview"

# SUMMARY.md 업데이트
python .github/scripts/devlog/update_summary.py Documents
```

## 커스터마이징

### 템플릿 수정

각 템플릿 파일을 수정하여 프로젝트에 맞게 커스터마이징:

- `daily_template.md` - Daily DevLog 형식
- `weekly_template.md` - Weekly Report 형식
- `system_review_template.md` - System Review 형식

### 스케줄 변경

`.github/workflows/*.yml` 파일의 `cron` 표현식을 수정:

```yaml
on:
  schedule:
    - cron: '0 0 * * *'  # 매일 UTC 00:00 (KST 09:00)
```

### 메트릭 추가

Python 스크립트에서 다음 함수들을 확장:

- `parse_ubt_summary()` - 빌드 메트릭
- `parse_cook_summary()` - 쿠킹 메트릭
- `parse_tests()` - 테스트 메트릭
- `parse_static()` - 정적분석 메트릭

## 파일 구조

```
프로젝트/
├── Documents/
│   ├── book.json              # HonKit 설정
│   ├── README.md              # HonKit 메인 페이지
│   ├── SUMMARY.md             # HonKit 목차 (자동 생성)
│   ├── DevLog/                # 일일/주간 로그
│   ├── Planning/              # 기획 문서
│   └── SystemReview/          # 시스템 리뷰
│
├── .github/
│   ├── workflows/
│   │   ├── honkit.yml
│   │   ├── devlog-simple.yml
│   │   ├── weekly-report.yml
│   │   └── system-review.yml
│   │
│   └── scripts/devlog/
│       ├── generate_daily.py
│       ├── generate_weekly.py
│       ├── generate_system_review.py
│       ├── update_summary.py
│       ├── send_discord.py
│       ├── daily_template.md
│       ├── weekly_template.md
│       └── system_review_template.md
│
└── README.md                  # 프로젝트 README
```

## 문제 해결

### HonKit 빌드 실패

1. `book.json`의 플러그인 설정 확인
2. SUMMARY.md의 파일 경로가 올바른지 확인
3. Markdown 파일의 문법 오류 확인

### DevLog 생성 실패

1. Git 히스토리가 충분한지 확인 (`fetch-depth: 0`)
2. Python 의존성이 설치되었는지 확인
3. 템플릿 파일 경로 확인

### Discord 알림 미전송

1. `DISCORD_WEBHOOK_URL` Secret이 설정되었는지 확인
2. Webhook URL이 유효한지 Discord에서 확인
3. GitHub Actions 로그에서 에러 메시지 확인

## 고급 기능

### Doxygen 연동

코드 API 문서(Doxygen)와 DevLog(HonKit)를 분리:

- Doxygen: `docs/doxygen/`
- HonKit: `docs/` (또는 `docs/devlog/`)

### CI/CD 파이프라인 통합

빌드/테스트 결과를 DevLog에 자동 포함:

1. 빌드 결과를 JSON으로 저장
2. `--ubt_log`, `--test_xml` 인자로 전달
3. 템플릿에서 메트릭 표시

### 다국어 지원

`book.json`의 `language` 설정 변경:

```json
{
  "language": "en"  // or "ja", "zh", etc.
}
```

템플릿 파일도 해당 언어로 번역

## 참고 자료

- [HonKit 공식 문서](https://github.com/honkit/honkit)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [GitHub Actions 문서](https://docs.github.com/en/actions)
- [Discord Webhook 가이드](https://discord.com/developers/docs/resources/webhook)

## 라이선스

이 스킬은 MIT 라이선스로 제공됩니다.

---

**생성된 문서 예시**: [YiSan DevLog](https://doppleddiggong.github.io/YiSan/docs/)
