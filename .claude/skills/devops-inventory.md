# DevOps Inventory Skill

목적: 저장소 내 DevOps 구성 요소(워크플로, 스크립트, 훅, 설정, 배포 경로)를 자동으로 스캔하고 문서화하여 `Documents/Planning/DevOps_Inventory.md`를 생성/업데이트한다.

---

## 트리거(명령)
- `/devops-inventory` — 기본 스캔 후 문서 생성/업데이트
- `/devops-inventory scan` — 스캔만 수행, 변경사항을 요약 출력(파일 미작성)
- `/devops-inventory fix-summary` — HonKit SUMMARY 재생성(`.github/scripts/generate_summary.py` 실행)
- 옵션 입력(자연어/플래그 모두 허용):
  - `--target Documents/Planning/DevOps_Inventory.md` (기본 경로)
  - `--include-disabled` (비활성 워크플로도 문서화)
  - `--commit` (변경 파일을 커밋)

---

## 출력(목표 파일)
- 경로: `Documents/Planning/DevOps_Inventory.md`
- 요구 섹션(고정 헤더):
  1. `전체 구조 요약`
  2. `GitHub Actions 워크플로`
  3. `로컬 스크립트/도구`
  4. `구성 파일 요약`
  5. `배포 대상/경로`
  6. `필요한 시크릿/권한`
  7. `부재/비활성 항목(현황)`
  8. `운영 가이드(요약)`
  9. `링크`
- 문서 갱신은 **멱등적**이어야 하며, 동일 제목의 섹션을 덮어쓴다.

---

## 스캔 가이드
가능하면 `rg`(ripgrep)를 사용하고, 없으면 `find`/`grep`으로 대체한다. 경로/파일명 패턴:

### 1) GitHub Actions / CI
- 디렉토리: `.github/workflows/`
- 관련 파일: `*.yml`, `*.yaml`, `ai-review.disabled`, `deploy-index.yml`, `doxygen.yml`, `honkit.yml`, `daily-report.yml`, `weekly-report.yml`
- 기타 설정: `.github/config.yml`, `.github/gh-pages-index.html`

### 2) 문서/정적 사이트
- HonKit: `Documents/book.json`, `Documents/HonkitPage/book.json`, `Documents/SUMMARY.md`
- 생성기: `.github/scripts/generate_summary.py`, `.github/scripts/install_honkit.sh`, `.github/scripts/package.json`

### 3) Doxygen
- 설정: `Doxyfile`
- 로컬 실행: `Tools/Doxygen/RunDoxygen.ps1`
- 페이지 원본: `Documents/DoxygenPage/*.md`

### 4) DevLog 자동화
- 스크립트: `.github/scripts/devlog/*` (daily/weekly, feedback, 템플릿, Discord 전송)
- 워크플로: `daily-report.yml`, `weekly-report.yml`
- 구성: `.github/config.yml`
- 로컬 래퍼: `Tools/DevLog/run_devlog.sh`, `Tools/RunDevLogPy.cmd`, `Tools/DevLog/run_generate_daily_devlog_once.py`, `Tools/run_generate_daily_devlog_once.ps1`
- 테스트: `Tools/Tests/Test-DevLog.ps1`

### 5) 커밋 메시지 자동화/훅
- 생성기: `Tools/CommitMessage/gen_commit_msg.py`
- 훅 설치: `Tools/CommitMessage/install_commit_hook.py` (설치 대상: `.git/hooks/prepare-commit-msg`)
- 템플릿/래퍼: `Tools/CommitMessage/commit_template.md`, `Tools/CommitMessage/run_gen_commit_msg.sh`, `Tools/RunGenCommitMsgPy.cmd`
- 프로젝트 초기화: `Tools/SetupProject.cmd`

### 6) 기타/테스트/알림
- Discord: `.github/scripts/devlog/send_discord.py`
- 추가 테스트: `Tools/Tests/Test-AgentQA.ps1`

### 7) 부재 여부 확인(명시)
- 사전 훅/린트: `.pre-commit-config.*`, `commitlint`, `husky`
- 포매터/린터: `.clang-format`, `.editorconfig`, `.gitattributes`
- 빌드/컨테이너: `Dockerfile`, `Makefile`, `CMake*`

권장 검색 예시:
```bash
# GitHub Actions
ls -la .github/workflows || true

# 핵심 파일 존재 확인
test -f Doxyfile && echo Doxygen:OK || true
[ -d .github/scripts/devlog ] && echo DevLogScripts:OK || true

# 부재 항목 체크(예: pre-commit)
ls -1 . | grep -E "^\.pre-commit-config\.(yml|yaml)$" || true
```

---

## 문서 작성 규칙
- 모든 경로/파일/명령은 백틱(`)으로 감싼다.
- 각 항목은 한 줄 요약 후, 필요시 세부 경로를 불릿으로 나열한다.
- Secrets 이름만 언급하고, 값은 절대 노출하지 않는다.
- 실제 배포 경로(GitHub Pages)는 서술한다: `/docs`, `/doxygen`, `/`(index)
- 비활성 워크플로는 “비활성(파일 존재)”로 표기한다.

---

## HonKit 반영
문서 작성/갱신 후 HonKit 목차를 동기화한다.

```bash
python .github/scripts/generate_summary.py
```

필요 시 GitHub Actions의 `Generate HonKit` 워크플로를 `workflow_dispatch`로 수동 트리거한다.

---

## 커밋(옵션)
변경 사항을 커밋해야 할 경우, Conventional Commit을 따른다.

예시:
```
docs(devops): add or update DevOps inventory

- Add sections: overview, workflows, tools, secrets
- Sync HonKit SUMMARY
```

이미 `prepare-commit-msg` 훅이 설치되어 있다면 제목/요약은 자동 생성될 수 있다. 강제 비활성화는 `SKIP_AUTO_COMMIT_MSG=1` 환경변수로 가능.

---

## 수용 기준(AC)
- `Documents/Planning/DevOps_Inventory.md`가 존재하며, 고정 헤더 9개 섹션이 모두 포함된다.
- 각 섹션에 실제 파일 경로나 워크플로 파일명이 최소 1개 이상 나열된다.
- Secrets 항목에 `GITHUB_TOKEN`, `OPENAI_API_KEY`, `DISCORD_WEBHOOK`가 포함된다.
- HonKit SUMMARY가 최신 상태다.
- 재실행 시 중복 섹션이 생기지 않는다(멱등성).

---

## 예시 출력 스니펫
```markdown
## 1) 전체 구조 요약
- DevLog 자동화 → docs 브랜치 커밋 → HonKit 빌드 → Pages 배포
- Doxygen → Pages `/doxygen` 배포

## 2) GitHub Actions 워크플로
- `.github/workflows/daily-report.yml`
- `.github/workflows/weekly-report.yml`
- `.github/workflows/honkit.yml`
- `.github/workflows/doxygen.yml`
- `.github/workflows/deploy-index.yml`
- `.github/workflows/ai-review.disabled` (비활성)

## 6) 필요한 시크릿/권한
- `secrets.GITHUB_TOKEN`
- `secrets.OPENAI_API_KEY`
- `secrets.DISCORD_WEBHOOK`
```

---

## 주의사항
- Secrets 값/토큰/개인정보는 절대 로그/문서에 포함하지 않는다.
- 대용량 리포지토리에서의 전체 텍스트 검색은 제한적으로 수행한다(우선 경로 기반 확인 → 필요 시 파일 열람).
- 문서 외 파일은 의도치 않게 수정하지 않는다.

---

## 참고 링크
- 기존 문서: `Documents/Planning/DevOps_Inventory.md`
- HonKit: `Documents/book.json`, `Documents/SUMMARY.md`
- Doxygen: `Doxyfile`, `Tools/Doxygen/RunDoxygen.ps1`
- DevLog 스크립트: `.github/scripts/devlog/`
- Commit 메시지 자동화: `Tools/CommitMessage/`
