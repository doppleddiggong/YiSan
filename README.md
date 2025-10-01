# YiSan 프로젝트

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6.0-blue.svg)](https://www.unrealengine.com/)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/Docs-GitHub%20Pages-lightgrey.svg)](https://doppleddiggong.github.io/YiSan/)

차세대 언리얼 엔진 기반 역사 체험형 게임 **YiSan**의 핵심 코드와 툴링을 담은 레포지토리입니다.

![Game Title](https://github.com/doppleddiggong/YiSan/blob/main/Documents/Reference/Title.png?raw=true)

[![Wiki](https://img.shields.io/badge/Documentation-Wiki-lightgrey.svg)](https://github.com/doppleddiggong/YiSan/wiki)

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

## 폴더 구조 스냅샷
```
Source/
 ├─ YiSan/                # 게임 모듈 (환경, 진입 로직)
 ├─ LatteLibrary/         # 캐릭터/전투/네트워크/데이터 서브시스템
 ├─ CoffeeLibrary/        # 공용 액터, 기능, 공유 유틸
Plugins/
 └─ CoffeeToolbar/        # 에디터 전용 툴바 플러그인
Documents/
 └─ DevLog/               # 일일 DevLog 및 30일 요약 (자동 생성)
Tests/                     # PowerShell 기반 검증 스크립트
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

---
최신 개발 내용은 항상 `Documents/DevLog` 디렉터리의 최근 30일 기록을 참고해 주세요.
