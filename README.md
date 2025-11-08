# YiSan 프로젝트

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6.0-blue.svg)](https://www.unrealengine.com/)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/Docs-Doxygen-lightgrey.svg)](https://doppleddiggong.github.io/YiSan/doxygen/)
[![DevLog](https://img.shields.io/badge/DevLog-HonKit-orange.svg)](https://doppleddiggong.github.io/YiSan/docs/)

차세대 언리얼 엔진 기반 역사 체험형 게임 **YiSan**의 핵심 코드와 툴링을 담은 레포지토리입니다.

![Game Title](https://github.com/doppleddiggong/YiSan/blob/main/Documents/Reference/Title.png?raw=true)

<h2>✓ 최종 결과물</h2>

<div align="left">
  🎥 <a href="https://youtu.be/BpUWFCA6qvw">프로토 구현</a><br>
  🎥 <a href="https://youtu.be/NPsN4mELEWA">알파 구현</a><br>
  🎥 <a href="https://youtu.be/9KfzKO5UqXw">베타 구현</a>
</div>


## 프로젝트 개요
- **엔진 버전**: Unreal Engine 5.6 (C++20)
- **핵심 목표**: 고밀도 NPC 상호작용과 실시간 이벤트 연동, 커스텀 라이브러리를 통한 게임플레이 확장
- **주요 모듈**
  - `YiSan` (Runtime): 게임 실행 진입점 및 환경 세팅
  - `LatteLibrary` (Runtime): 캐릭터/전투/네트워크 시스템
  - `CoffeeLibrary` (Runtime): 공용 유틸리티와 시퀀스/이펙트 파이프라인
  - `CoffeeToolbar` (Editor): 에디터 툴바 확장 플러그인

## 최근 30일 개발 하이라이트 *(2025-09-30 DevLog 기준)*
- 빌드 의존성 체인 정리 및 PowerShell 기반 회귀 검증 스크립트 추가
- `FCharacterInfoData.csv` 샘플 및 구조적 로그 검증 루틴으로 MasterData 재임포트 안정화
- `APlayerActor`에 SpringArm + FollowCamera 구성 적용, 3인칭 기본 뷰 정비
- Dev/Test 스크립트 표준화: RowName 검증, Attempt/Outcome 구조 로그 강제

## 문서화 시스템

이 프로젝트는 **이중 문서화 시스템**을 사용합니다:

### 📚 Doxygen - 코드 API 문서
- **URL**: [https://doppleddiggong.github.io/YiSan/doxygen/](https://doppleddiggong.github.io/YiSan/doxygen/)
- **내용**: C++ 클래스, 함수, 변수에 대한 자동 생성 API 문서
- **업데이트**: 코드 커밋 시 자동으로 빌드 및 배포

### 📝 HonKit - 개발 문서
- **URL**: [https://doppleddiggong.github.io/YiSan/docs/](https://doppleddiggong.github.io/YiSan/docs/)
- **내용**:
  - **DevLog**: 일일/주간/월간 개발 로그 (자동 생성)
  - **Planning**: 시스템 설계, 기능 명세, 발표 자료
  - **System Review**: 아키텍처 변화 분석 및 성능 메트릭
- **업데이트**: DevLog/Planning 폴더 변경 시 자동으로 빌드 및 배포

## 폴더 구조 스냅샷
```
Source/
 ├─ YiSan/                # 게임 모듈 (환경, 진입 로직)
 ├─ LatteLibrary/         # 캐릭터/전투/네트워크/데이터 서브시스템
 ├─ CoffeeLibrary/        # 공용 액터, 기능, 공유 유틸
Plugins/
 └─ CoffeeToolbar/        # 에디터 전용 툴바 플러그인
Documents/
 ├─ DevLog/               # 일일 DevLog 및 주간/월간 요약 (자동 생성)
 ├─ Planning/             # 시스템 설계 및 기획 문서
 ├─ book.json             # HonKit 설정
 └─ SUMMARY.md            # HonKit 목차 (자동 생성)
Tests/                     # PowerShell 기반 검증 스크립트
.github/
 ├─ workflows/
 │  ├─ doxygen.yml        # Doxygen 자동 빌드
 │  ├─ honkit.yml         # HonKit 자동 빌드
 │  └─ devlog-simple.yml  # Daily DevLog 자동 생성
 └─ scripts/devlog/       # DevLog 생성 스크립트
```

## 빌드 및 실행
1. Epic Games Launcher 또는 BuildGraph로 **UE 5.6** 툴체인을 준비합니다.
2. `YiSan.uproject` 우클릭 후 *Generate Visual Studio project files* (또는 Rider) 실행
3. `YiSanEditor` 타깃을 Development/Win64로 빌드
4. 에디터 실행 후 `YiSan` 맵을 로드해 기능 확인

### PreBuild DevLog 자동화
- `.uproject`에 `PreBuildSteps`를 추가하면 빌드 직전 `Tools\run_generate_daily_devlog_once.ps1`이 실행되어 최근 활동을 DevLog에 기록합니다.
- DevLog 출력: `Documents/DevLog/YYYY-MM-DD.md` 및 `_Last30Summary.md`

## 테스트 스크립트
| 경로 | 목적 |
| ---- | ---- |
| `Tests/ModuleDependency.Tests.ps1` | 모듈 공개/비공개 의존성 검증 |
| `Tests/PlayerActor.Tests.ps1` | SpringArm/Camera 구성 및 회전 설정 검증 |
| `Tests/MasterData.Tests.ps1` | MasterData CSV 포맷 및 RowName 유효성 검사 |

> 모든 스크립트는 Attempt/Outcome 구조 로그 형식을 따르며, 실패 시 비구조적 로그 사용을 금지합니다.

## MasterData 워크플로우
1. `Documents/MasterData/FCharacterInfoData.csv` 편집 (헤더: `Name,Type,HP,ATK,SightLength,SightAngle`)
2. PowerShell 테스트 실행: `powershell -ExecutionPolicy Bypass -File Tests/MasterData.Tests.ps1`
3. Unreal Editor에서 해당 데이터테이블 재임포트 후 런타임 캐시 확인

## 향후 계획
- `UBT -ListActions` 기반 빌드 액션 수치화 및 성능 보고서 자동화
- DevLog 생성 파이프라인을 CI에 통합해 팀 공유 강화
- MasterData/Subsystem 캐시 검증을 통합 테스트로 확장

## 기여 가이드
1. PR 전 PowerShell 테스트 스크립트를 모두 통과시킵니다.
2. 구조적 로그 규칙(CorrelationId, Operation, Attempt, Outcome 등)을 위반하지 않습니다.
3. 변경 사항을 DevLog에 반영하고, 필요한 경우 `_Last30Summary.md`를 갱신합니다.

## 자동화 시스템

프로젝트는 6개의 GitHub Actions 워크플로우로 자동화되어 있습니다.

### 📊 CI/CD 워크플로우 개요

| 워크플로우 | 트리거 | 주기 | 설명 |
|-----------|--------|------|------|
| **Doxygen** | 코드 푸시/PR | 실시간 | C++ API 문서 자동 생성 및 배포 |
| **HonKit** | DevLog/Planning 변경 | 실시간 | 개발 문서 사이트 빌드 및 배포 |
| **Daily DevLog** | Cron + Manual | 매일 오전 8:30 (KST) | 일일 개발 로그 자동 생성 (OpenAI) |
| **Daily DevLog (Simple)** | Cron + Manual | 매일 오전 9:00 (KST) | 일일 개발 로그 자동 생성 (Python) |
| **Weekly Report** | Cron + Manual | 매주 일요일 23:00 (KST) | 주간 개발 요약 리포트 생성 |
| **System Review** | Cron + Manual | 매월 1일 23:00 (KST) | 월간 시스템 리뷰 생성 |

### 1. Doxygen 문서화
- **파일**: `.github/workflows/doxygen.yml`
- **트리거**: `Source/`, `Plugins/`, `*.md`, `Doxyfile` 변경 시
- **출력**: [API 문서](https://doppleddiggong.github.io/YiSan/doxygen/)
- **기능**: Doxygen Awesome CSS 테마 적용, GraphViz 다이어그램 생성

### 2. HonKit 문서 사이트
- **파일**: `.github/workflows/honkit.yml`
- **트리거**: `Documents/DevLog/`, `Documents/Planning/` 변경 시
- **출력**: [개발 문서](https://doppleddiggong.github.io/YiSan/docs/)
- **플러그인**:
  - `mermaid-gb3`: 다이어그램 렌더링
  - `collapsible-menu`: 접을 수 있는 메뉴
  - `search-plus`: 향상된 검색
  - `expandable-chapters-small`: 챕터 확장/축소

### 3. Daily DevLog (OpenAI 기반)
- **파일**: `.github/workflows/devlog.yml`
- **스케줄**: 매일 오전 8:30 (KST) - `cron: '30 23 * * *'`
- **기능**:
  - OpenAI GPT-4 Turbo로 커밋 분석 및 DevLog 자동 생성
  - Pull Request 자동 생성
  - Discord 웹훅 알림
- **수동 실행**: 날짜 지정 및 강제 생성 가능
- **환경 변수**: `OPENAI_API_KEY`, `DISCORD_WEBHOOK_URL`

### 4. Daily DevLog (Python 기반)
- **파일**: `.github/workflows/devlog-simple.yml`
- **스케줄**: 매일 오전 9:00 (KST) - `cron: '0 0 * * *'`
- **생성 위치**: `Documents/DevLog/YYYY-MM-DD.md`
- **수집 정보**:
  - Git 커밋 통계 (Conventional Commits 기반)
  - 빌드 및 테스트 결과
  - 코드 변경 분석
- **자동 커밋**: GitHub Actions Bot이 직접 커밋
- **템플릿**: `.github/scripts/devlog/daily_template.md`

### 5. Weekly Report
- **파일**: `.github/workflows/weekly-report.yml`
- **스케줄**: 매주 일요일 23:00 (KST) - `cron: '0 14 * * 0'`
- **생성 위치**: `Documents/DevLog/YYYY-W##-Summary.md`
- **기능**:
  - 주간 커밋 통계 및 트렌드 분석
  - 주요 작업 하이라이트
  - Discord 알림 (선택)
- **템플릿**: `.github/scripts/devlog/weekly_template.md`

### 6. System Review
- **파일**: `.github/workflows/system-review.yml`
- **스케줄**: 매월 1일 23:00 (KST) - `cron: '0 14 1 * *'`
- **생성 위치**: `Documents/SystemReview/YYYY-MM-SystemReview.md`
- **기능**:
  - 아키텍처 변화 분석
  - 성능 메트릭 추적
  - 코드 품질 리뷰
  - 주간/월간 리뷰 타입 선택 가능
- **템플릿**: `.github/scripts/devlog/system_review_template.md`

### Discord 알림 설정 (선택)
Discord 웹훅을 설정하면 DevLog 생성 시 자동으로 알림을 받을 수 있습니다:

1. Discord 서버에서 웹훅 URL 생성
2. GitHub Repository Settings → Secrets → Actions
3. `DISCORD_WEBHOOK_URL` 시크릿 추가

### 수동 실행 방법
모든 워크플로우는 수동 실행(`workflow_dispatch`)을 지원합니다:

```bash
# GitHub CLI 사용
gh workflow run devlog-simple.yml
gh workflow run weekly-report.yml --field date=2025-01-15
gh workflow run system-review.yml --field type=weekly
```

또는 GitHub 웹 UI:
1. Actions 탭 이동
2. 원하는 워크플로우 선택
3. "Run workflow" 버튼 클릭

---

**📚 최신 개발 내용**: [HonKit DevLog](https://doppleddiggong.github.io/YiSan/docs/)
**🔍 API 문서**: [Doxygen Documentation](https://doppleddiggong.github.io/YiSan/doxygen/)
