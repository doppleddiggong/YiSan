# YiSan DevOps 개요 문서

작성일: 2025-11-09

본 문서는 YiSan 저장소에 구성된 DevOps(자동화/문서화/훅) 기능을 한눈에 파악하고 운영·유지보수 시 참고할 수 있도록 정리합니다.

## 1) 전체 구조 요약
- DevLog 자동화: 매일/매주 커밋 기반 개발 로그 생성 → `docs` 브랜치에 커밋 → HonKit 빌드 트리거 → GitHub Pages 배포
- HonKit 문서: `Documents` 폴더를 원본으로 빌드하여 GitHub Pages(`/docs`)에 배포
- Doxygen 문서: C++ 소스에 대해 Doxygen 생성 후 GitHub Pages(`/doxygen`) 배포
- 커밋 메시지 자동화: `prepare-commit-msg` 훅으로 Conventional Commit 스타일 제목/요약 자동 생성
- 인덱스 페이지: GitHub Pages 루트 인덱스(`/.`) 배포
- AI 코드 리뷰 워크플로: 존재하지만 비활성(수동 활성화 가능)

## 2) GitHub Actions 워크플로

### 2.1 Daily Report (일간 DevLog)
- 파일: `.github/workflows/daily-report.yml`
- 트리거: 매일 09:00 KST(UTC 00:00), 수동 실행 지원
- 동작:
  - `docs` 브랜치 체크아웃 → Python 환경 설정 → 커밋 로그 기준 일간 DevLog 생성
  - 선택: GPT 피드백 생성/병합(`.github/config.yml`의 `gpt.daily_enabled` 및 워크플로 입력)
  - `Documents/DevLog/Daily/YYYY-MM-DD.md` 및 메트릭 JSON 생성
  - `Documents/SUMMARY.md` 갱신 → `docs` 브랜치에 커밋/푸시
  - Discord 알림(선택, `DISCORD_WEBHOOK` 설정 필요)
  - HonKit 빌드 트리거(repository_dispatch: `honkit-build`)

### 2.2 Weekly Report (주간 요약/GPT 회고)
- 파일: `.github/workflows/weekly-report.yml`
- 트리거: 매주 일요일 23:00 KST(UTC 14:00), 수동 실행 지원
- 동작:
  - `docs` 브랜치 체크아웃 → Python 설정 → 주간 요약 생성(`WYY` 레이블)
  - 선택: GPT 기반 주간 피드백 생성/병합(`.github/config.yml`의 `gpt.weekly_enabled`)
  - `Documents/SUMMARY.md` 갱신 → `docs` 브랜치 커밋/푸시 → Discord(선택) → HonKit 트리거

### 2.3 HonKit 빌드/배포
- 파일: `.github/workflows/honkit.yml`
- 트리거:
  - `main/dopple/docs` 브랜치의 `Documents/**` 및 `generate_summary.py` 변경 시
  - `repository_dispatch: honkit-build`, 수동 실행
- 동작:
  - `Documents` 루트 기준으로 SUMMARY 생성 → HonKit 설치/빌드 → Pages `gh-pages/docs`에 배포

### 2.4 Doxygen 생성/배포
- 파일: `.github/workflows/doxygen.yml`
- 트리거: `main` 브랜치의 `Source/**`, `Plugins/**`, `**/*.md`, `Doxyfile` 변경 및 PR
- 동작: Doxygen/Graphviz 설치 → 문서 생성 → Pages `gh-pages/doxygen`에 배포

### 2.5 GitHub Pages 인덱스 배포
- 파일: `.github/workflows/deploy-index.yml`
- 트리거: `main`의 인덱스 관련 파일 변경, 수동 실행
- 동작: `.github/gh-pages-index.html`을 Pages 루트(`/.`)로 배포

### 2.6 AI 코드 리뷰(비활성)
- 파일: `.github/workflows/ai-review.disabled`
- 개요: 변경된 C++/C# diff를 OpenAI API로 리뷰, PR 코멘트/Discord 전송
- 비고: 기본 비활성. 활성화 시 OpenAI API Key 필요

## 3) 로컬 스크립트/도구

### 3.1 DevLog 관련
- 실행 래퍼: `Tools/DevLog/run_devlog.sh`, `Tools/RunDevLogPy.cmd`
- 1회 생성/백필: `Tools/DevLog/run_generate_daily_devlog_once.py`, `Tools/run_generate_daily_devlog_once.ps1`
- 기타: `Tools/DevLog/generate_daily_devlog_bi.py`
- 테스트: `Tools/Tests/Test-DevLog.ps1`

### 3.2 커밋 메시지 자동화
- 메시지 생성: `Tools/CommitMessage/gen_commit_msg.py`
- 훅 설치: `Tools/CommitMessage/install_commit_hook.py` → `.git/hooks/prepare-commit-msg` 생성
- 템플릿/래퍼: `Tools/CommitMessage/commit_template.md`, `Tools/CommitMessage/run_gen_commit_msg.sh`, `Tools/RunGenCommitMsgPy.cmd`
- 초기화: `Tools/SetupProject.cmd` (Python 확인 → 훅 설치 → DevLog 1회 실행)
- 환경변수: `SKIP_AUTO_COMMIT_MSG=1` 설정 시 자동 생성 비활성

### 3.3 Doxygen 로컬 실행
- 실행: `pwsh -File Tools/Doxygen/RunDoxygen.ps1 [-Clean] [-ConfigPath <Doxyfile>]`
- 산출물: `Documents/Doxygen` (로컬 기준)

### 3.4 기타
- Agent Q&A 저장: `Tools/SaveAgentQA/save_agent_qa.py`, 테스트 `Tools/Tests/Test-AgentQA.ps1`

## 4) 구성 파일 요약
- DevLog/알림 중앙 설정: `.github/config.yml`
  - `gpt.daily_enabled`, `gpt.weekly_enabled` (GPT 사용 여부)
  - `discord.enabled` (Webhook 사용)
  - `devlog.daily_auto`, `devlog.weekly_auto`, `output_dir`
- HonKit 설정: `Documents/book.json`, `Documents/HonkitPage/book.json`
- Doxygen 설정: `Doxyfile`

## 5) 배포 대상/경로
- GitHub Pages 브랜치: `gh-pages`
  - HonKit: `/docs`
  - Doxygen: `/doxygen`
  - 인덱스: `/`

## 6) 필요한 시크릿/권한
- `secrets.GITHUB_TOKEN`: Pages 배포/푸시용(기본 제공)
- `secrets.OPENAI_API_KEY`: GPT 기반 일간/주간 피드백, AI 코드 리뷰(선택)
- `secrets.DISCORD_WEBHOOK`: Discord 알림 활성화 시 필수

## 7) 부재/비활성 항목(현황)
- Pre-commit 프레임워크 설정 없음: `.pre-commit-config.*` 미존재
- `commitlint`/`husky` 미사용
- 코드 포매터/린터 표준 파일 미검출: `.clang-format`, `.editorconfig`, `.gitattributes` 등
- Git 훅 커버리지는 `prepare-commit-msg`만 자동 설치(기타 훅 없음)
- Dockerfile/Makefile/CMake 등 빌드 시스템 정의 미검출

## 8) 운영 가이드(요약)
- 커밋 메시지 훅 설치: 
  - `python Tools/CommitMessage/install_commit_hook.py`
  - 또는 `Tools/SetupProject.cmd` 실행(권장)
- DevLog 수동 실행:
  - Windows: `Tools/RunDevLogPy.cmd`
  - Bash: `./Tools/DevLog/run_devlog.sh`
- HonKit 강제 빌드:
  - GitHub Actions에서 `Generate HonKit` 워크플로 `workflow_dispatch` 실행
  - 또는 Daily/Weekly 리포트 완료 시 자동 트리거
- Doxygen 로컬 생성:
  - `pwsh -File Tools/Doxygen/RunDoxygen.ps1 -Clean`

## 9) 링크
- 저장소: https://github.com/doppleddiggong/YiSan
- 문서 사이트(HonKit): https://doppleddiggong.github.io/YiSan/docs/
- API 문서(Doxygen): https://doppleddiggong.github.io/YiSan/doxygen/

