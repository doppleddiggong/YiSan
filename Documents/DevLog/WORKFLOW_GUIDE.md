# 프로젝트 자동화 워크플로우 가이드

## 1. 개요

이 프로젝트는 GitHub Actions를 사용하여 문서화, 개발 로그 작성, 코드 리뷰 등 반복적인 작업을 자동화합니다. 이를 통해 개발자는 핵심 로직 개발에 집중하고, 프로젝트의 모든 기록은 체계적으로 관리됩니다.

---

## 2. 주요 워크플로우

### 2.1. Doxygen - C++ API 문서 자동화

- **워크플로우 파일:** `.github/workflows/doxygen.yml`
- **실행 트리거:** `Source/` 또는 `Plugins/` 디렉토리의 C++ 코드 변경 시
- **주요 기능:**
    1.  C++ 소스 코드의 Doxygen 주석(`/** ... */`)을 파싱합니다.
    2.  클래스 다이어그램을 포함한 HTML 문서를 생성합니다.
    3.  생성된 문서를 `gh-pages` 브랜치에 자동으로 배포합니다.
- **결과물:** [Doxygen API 문서 사이트](https://doppleddiggong.github.io/YiSan/doxygen/)

### 2.2. HonKit - 개발 문서 사이트 자동화

- **워크플로우 파일:** `.github/workflows/honkit.yml`
- **실행 트리거:** `Documents/` 폴더 내의 마크다운(`.md`) 파일 변경 시
- **주요 기능:**
    1.  `Documents/` 폴더의 모든 마크다운 파일을 읽어들입니다.
    2.  HonKit(GitBook)을 사용하여 정적 웹사이트를 빌드합니다.
    3.  빌드된 사이트를 `gh-pages` 브랜치의 `/docs` 경로에 배포합니다.
- **결과물:** [개발 문서 사이트](https://doppleddiggong.github.io/YiSan/docs/)

### 2.3. DevLog - 개발 로그 자동화

- **워크플로우 파일:** `.github/workflows/devlog-simple.yml`, `weekly-report.yml`
- **실행 트리거:** 매일/매주 정해진 시간 (또는 수동 실행)
- **주요 기능:**
    - **Daily Log:** 매일 오전 9시(KST), Git 커밋 내역을 분석하여 그날의 개발 활동을 마크다운 파일로 자동 생성합니다.
    - **Weekly Summary:** 매주 일요일 밤, 한 주간의 개발 로그를 요약하여 주간 리포트를 생성합니다.
    - **AI 피드백 (선택):** 수동 실행 시, OpenAI GPT를 통해 코드 변경 사항에 대한 성찰 질문이나 대안을 제시하는 피드백을 받을 수 있습니다.
- **결과물:** `Documents/DevLog/` 폴더에 `YYYY-MM-DD.md` 및 `YYYY-WXX-Summary.md` 파일 생성.

### 2.4. AI Code Review - 코드 리뷰 자동화

- **워크플로우 파일:** `.github/workflows/ai-review.disabled` (현재 비활성화)
- **실행 트리거:** Pull Request 생성 시
- **주요 기능:**
    1.  PR의 코드 변경 사항(`git diff`)을 추출합니다.
    2.  OpenAI GPT API로 변경된 코드에 대한 리뷰를 요청합니다.
    3.  리뷰 결과를 PR 코멘트로 자동 등록합니다.
- **목적:** 동료 리뷰의 부담을 줄이고, 잠재적인 버그나 코드 스타일 문제를 조기에 발견하기 위한 실험적인 기능입니다.

---

## 3. 수동 실행 및 설정

### 수동 실행 방법
모든 워크플로우는 GitHub 저장소의 **Actions** 탭에서 수동으로 실행할 수 있습니다.
1.  원하는 워크플로우를 선택합니다.
2.  `Run workflow` 버튼을 클릭합니다.
3.  필요한 경우, `use_gpt`와 같은 옵션을 선택하여 실행할 수 있습니다.

### 필요 설정
- **GitHub Pages:** 저장소의 `Settings` &rarr; `Pages`에서 소스를 `gh-pages` 브랜치의 `/ (root)`로 설정해야 합니다.
- **Secrets:** AI 리뷰나 DevLog의 GPT 피드백 기능을 사용하려면, `Settings` &rarr; `Secrets and variables` &rarr; `Actions`에 `OPENAI_API_KEY`가 등록되어 있어야 합니다.