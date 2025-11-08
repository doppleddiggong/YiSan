# HonKit DevLog Automation Skill 사용 가이드

다른 프로젝트에 HonKit 기반 DevLog 자동화 시스템을 적용하는 방법입니다.

## 빠른 시작

### 방법 1: 설치 스크립트 사용 (권장)

```bash
# YiSan 저장소에서 스크립트 복사
curl -o install_honkit.sh https://raw.githubusercontent.com/doppleddiggong/YiSan/main/.github/scripts/install_honkit.sh

# 실행 권한 부여
chmod +x install_honkit.sh

# 스크립트 실행
./install_honkit.sh
```

스크립트가 다음을 자동으로 생성합니다:
- Documents 폴더 구조
- book.json 설정
- README.md 및 SUMMARY.md

### 방법 2: Claude Code Skill 사용

```bash
# Claude Code에서 실행
/skill honkit-devlog
```

Claude Code가 대화형으로 설정을 안내합니다.

### 방법 3: 수동 설정

YiSan 프로젝트에서 다음 파일들을 복사:

```bash
# 1. 스크립트 복사
cp -r YiSan/.github/scripts/devlog/ YOUR_PROJECT/.github/scripts/devlog/

# 2. 워크플로우 복사
cp YiSan/.github/workflows/honkit.yml YOUR_PROJECT/.github/workflows/
cp YiSan/.github/workflows/devlog-simple.yml YOUR_PROJECT/.github/workflows/
cp YiSan/.github/workflows/weekly-report.yml YOUR_PROJECT/.github/workflows/
cp YiSan/.github/workflows/system-review.yml YOUR_PROJECT/.github/workflows/

# 3. Documents 설정 복사
cp YiSan/Documents/book.json YOUR_PROJECT/Documents/
```

## 필요한 파일 목록

### Python 스크립트 (.github/scripts/devlog/)

| 파일 | 용도 | 크기 |
|------|------|------|
| `generate_daily.py` | Daily DevLog 생성 | ~10KB |
| `daily_template.md` | Daily 템플릿 | ~3KB |
| `generate_weekly.py` | Weekly Report 생성 | ~8KB |
| `weekly_template.md` | Weekly 템플릿 | ~5KB |
| `generate_system_review.py` | System Review 생성 | ~9KB |
| `system_review_template.md` | System Review 템플릿 | ~7KB |
| `update_summary.py` | SUMMARY.md 업데이트 | ~6KB |
| `send_discord.py` | Discord 알림 전송 | ~5KB |

### GitHub Actions 워크플로우 (.github/workflows/)

| 파일 | 용도 | 스케줄 |
|------|------|--------|
| `honkit.yml` | HonKit 빌드 및 배포 | DevLog 업데이트 시 |
| `devlog-simple.yml` | Daily DevLog 자동 생성 | 매일 오전 9시 (KST) |
| `weekly-report.yml` | Weekly Report 생성 | 매주 일요일 오후 11시 |
| `system-review.yml` | System Review 생성 | 매월 1일 오후 11시 |

## 프로젝트별 커스터마이징

### 1. book.json 수정

```json
{
  "title": "[프로젝트명] Development Documentation",
  "description": "[프로젝트 설명]",
  "author": "[작성자명]",
  "pluginsConfig": {
    "github": {
      "url": "https://github.com/[USERNAME]/[REPO]"
    }
  }
}
```

### 2. 워크플로우 스케줄 조정

`.github/workflows/*.yml` 파일의 `cron` 수정:

```yaml
on:
  schedule:
    # 매일 KST 오전 10시로 변경 예시
    - cron: '0 1 * * *'  # UTC 01:00 = KST 10:00
```

**시간대 변환 참고**:
- KST = UTC + 9시간
- KST 09:00 → UTC 00:00
- KST 18:00 → UTC 09:00

### 3. 템플릿 커스터마이징

템플릿 파일 수정:

```bash
# Daily DevLog 템플릿
vi .github/scripts/devlog/daily_template.md

# Weekly Report 템플릿
vi .github/scripts/devlog/weekly_template.md

# System Review 템플릿
vi .github/scripts/devlog/system_review_template.md
```

**커스터마이징 예시**:
- 섹션 추가/제거
- Mermaid 다이어그램 스타일 변경
- 회고 질문 수정

### 4. 프로그래밍 언어별 적용

#### Python 프로젝트
- 테스트 결과: pytest의 JUnit XML 출력 사용
- 정적분석: pylint, flake8 결과 수집

#### JavaScript/TypeScript 프로젝트
- 테스트 결과: Jest의 JUnit 리포터 사용
- 정적분석: ESLint 결과 수집

#### Java 프로젝트
- 테스트 결과: JUnit XML
- 정적분석: SonarQube, Checkstyle 결과

#### C++ 프로젝트 (Unreal Engine)
- 빌드 결과: UnrealBuildTool 로그 파싱
- 정적분석: clang-tidy, cppcheck 결과

## 고급 기능 활용

### 1. 메트릭 수집 추가

빌드/테스트 결과를 DevLog에 포함:

```python
# generate_daily.py 수정
def parse_your_build_log(path):
    # 빌드 로그 파싱 로직 추가
    return {
        "success": True,
        "duration": "5m 32s",
        "warnings": 3
    }
```

### 2. Doxygen 연동

API 문서 변화 추적:

```yaml
# .github/workflows/devlog-simple.yml
- name: Generate Doxygen
  run: doxygen Doxyfile

- name: Analyze API Changes
  run: |
    python .github/scripts/devlog/analyze_doxygen.py \
      --prev "docs/doxygen-prev" \
      --current "docs/doxygen" \
      --out "api-changes.json"
```

### 3. 성능 메트릭 수집

벤치마크 결과 추적:

```json
// Documents/DevLog/YYYY-MM-DD.metrics.json
{
  "load_time_prev": 2.5,
  "load_time_now": 2.1,
  "delta_pct": -16.0,
  "fps_prev": 55,
  "fps_now": 60
}
```

### 4. 다국어 지원

영어 버전 생성:

```bash
# book.json
{
  "language": "en",
  "root": "./docs/en"
}

# 템플릿도 영어로 번역
cp daily_template.md daily_template_en.md
# 내용을 영어로 번역
```

## 트러블슈팅

### HonKit 빌드 오류

**증상**: HonKit 빌드가 실패합니다.

**해결책**:
1. `book.json`의 JSON 문법 확인
2. 플러그인 이름 오타 확인
3. SUMMARY.md의 링크가 유효한지 확인

```bash
# 로컬 테스트
npm install -g honkit
cd Documents
honkit build
```

### Python 스크립트 오류

**증상**: DevLog 생성 스크립트가 실패합니다.

**해결책**:
1. Python 버전 확인 (3.11+ 필요)
2. 의존성 설치 확인
3. 템플릿 파일 경로 확인

```bash
# 로컬 테스트
pip install jinja2 pyyaml
python .github/scripts/devlog/generate_daily.py --help
```

### GitHub Actions 권한 오류

**증상**: 워크플로우가 커밋/푸시할 수 없습니다.

**해결책**:
1. Repository Settings → Actions → General
2. Workflow permissions → Read and write permissions 선택
3. Save

### Discord 알림 실패

**증상**: Discord 메시지가 전송되지 않습니다.

**해결책**:
1. Webhook URL이 올바른지 확인
2. GitHub Secret 이름 확인 (`DISCORD_WEBHOOK_URL`)
3. Discord 채널 권한 확인

## 예시 프로젝트

다음 프로젝트들이 HonKit DevLog Skill을 사용하고 있습니다:

| 프로젝트 | 언어 | DevLog URL |
|---------|------|-----------|
| **YiSan** | C++ (UE5) | [Docs](https://doppleddiggong.github.io/YiSan/docs/) |

*이 리스트에 프로젝트를 추가하려면 PR을 보내주세요!*

## 기여하기

Skill 개선에 기여하려면:

1. YiSan 저장소 Fork
2. 개선 사항 구현
3. Pull Request 생성

## 라이선스

MIT License - 자유롭게 사용하고 수정할 수 있습니다.

## 지원

문제가 발생하면:

1. [GitHub Issues](https://github.com/doppleddiggong/YiSan/issues)에 보고
2. [Discussions](https://github.com/doppleddiggong/YiSan/discussions)에서 질문

---

**이 가이드로 30분 안에 DevLog 자동화를 설정할 수 있습니다!** 🚀
