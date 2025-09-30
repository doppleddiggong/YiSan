## 6. DragonBallSZ — 프로젝트 개요

### 1) 개요
- **프로젝트**: DragonBallSZ (Dragon Ball Sparking Zero Replica)
- **엔진**: Unreal Engine **5.6.0**  
  - `DragonBallSZ.uproject` → `EngineAssociation: "5.6"`
- **유형**: C++ 기반 게임 프로젝트 + 자체 유틸리티 모듈 + 에디터 플러그인
- **주요 기능**: 캐릭터 전투(대시/부스트/넉백/히트스탑/러시어택), AI, 카메라 매니저, 데이터·이벤트 관리, 전투 UI, 나이아가라 VFX, Enhanced Input

---

### 2) 엔진 / 툴체인
- **UE 버전**: 5.6  
  - `IncludeOrderVersion: Unreal5_6`  
  - `BuildSettingsVersion: V5`
- **C++ 표준**: C++20
- **IDE/툴**: JetBrains Rider
- **VS 컴포넌트(.vsconfig)**  
  - Windows 11 SDK 22621  
  - VC++ 툴체인  
  - LLVM/Clang  
  - Unreal VS 통합  
  - Native Game 워크로드

---

### 3) 언어 / 런타임
- **C++**: 엔진 모듈 / 게임 로직
- **블루프린트**: 일부 사용 (기본 게임모드 `GM_DBSZ` 블루프린트)
- **UI**: UMG / Slate (런타임 UI 및 에디터 UI)
- **VFX**: Niagara

---

### 4) 모듈 구성

#### 4-1) 게임 모듈: `DragonBallSZ` (Type: Runtime)
- **PublicDeps**: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `Niagara`, `AIModule`, `GameplayTasks`, `NavigationSystem`, `CoffeeLibrary`
- **디렉터리 구조**: `Character`, `Enviroment`, `Common`, `MasterData`, `UI`
- **로그 카테고리**: `DBSZ` (`DECLARE_LOG_CATEGORY_EXTERN` / `DEFINE_LOG_CATEGORY`)

#### 4-2) 유틸 모듈: `CoffeeLibrary` (Type: Runtime)
- **PublicDeps**: `Core`, `CoreUObject`, `Engine`
- **PrivateDeps**: `InputCore`, `EnhancedInput`, `UMG`, `Slate`, `SlateCore`, `ApplicationCore`
- **Editor 빌드 시 추가**: `UnrealEd`, `EditorSubsystem`
- **주요 구성요소**
  - **Actor/Component**: `AListActorManager`, `UListActorComponent`, `UOrbitalBehaviorComponent`, `UTweenAnimInstance`
  - **Features**: `UEaseComponent/FunctionLibrary`, `UGameEventManager`, `UObjectPoolManager`, `USequenceManager`, `UParabolaComponent`, `UCommonFunctionLibrary`

#### 4-3) 에디터 플러그인: `CoffeeToolbar` (Type: Editor, Win64)
- **목적**: 에디터 툴바/설정 (Slate, ToolMenus, UnrealEd, LevelEditor, DeveloperSettings 등)
- **엔진 버전**: 5.6.0
- **Installed**: `true`
- **경로**: `Plugins/CoffeeToolbar/*`

---

### 5) 주요 서브시스템 (게임 모듈)
#### Character
- 캐릭터/AI: `APlayerActor`, `AEnemyActor`, `AEnemyAIController`, `UEnemyFSM`, `UEnemyAnimInstance`
- 전투/이동: `UDashSystem`, `UFlySystem`, `UKnockbackSystem`, `UHitStopSystem`, `URushAttackSystem`, `USightSystem`, `UStatSystem`

#### Enviroment
- 카메라/레벨: `ACameraManager`, `ADynamicCameraActor`, `ACombatLevelScript`, `ADBSZGameMode`

#### Common
- 공용: `UDBSZEventManager`, `GameEvent`, `GameColor`, 모듈 진입 `DragonBallSZ.cpp/h`

#### MasterData
- 데이터정의/매니저: `EAttackPowerType`, `EBodyPartType`, `FHitStopData`, `FKnockbackData`, `UDBSZDataManager`

#### UI
- 전투 UI: `UCombatUI` (UMG 연계)

---

### 6) 플랫폼 / 타깃
- **타깃**: `DragonBallSZTarget`(Game), `DragonBallSZEditorTarget`(Editor) — Win64
- **플러그인 화이트리스트**: `CoffeeToolbar` Editor 모듈 Win64 허용
- **AndroidFileServer 설정 존재**(`DefaultEngine.ini`) — 개발 편의 플러그인 활성화(실제 Android 타깃 여부와 무관)

---

### 7) 입력 / 맵 / 게임모드
- **Enhanced Input 사용**  
  - `DefaultPlayerInputClass = EnhancedPlayerInput`  
  - `DefaultInputComponentClass = EnhancedInputComponent`
- **Axis/Action 매핑**: WASD / 마우스 / 게임패드 기본 매핑
- **기본 맵/게임모드**
  - `EditorStartupMap` / `GameDefaultMap`: `/Game/CustomContents/Maps/DefaultLevel`
  - `GameMode`: `/Game/CustomContents/Blueprints/GM_DBSZ.GM_DBSZ_C`

---

### 8) 애셋 / 콘텐츠
- `Content/CustomContents/Assets`  
  - 캐릭터(Goku 등) 스켈레탈 메시, 애니메이션, 몽타주 다수
- Niagara 의존성 선언(이펙트 활용 전제)
- **대규모 바이너리 애셋** → Git 관리 시 **LFS 권장**

---

### 9) 빌드 / 실행

#### 솔루션
- `DragonBallSZ.sln` (UE 5.6 규격)

#### 명령행 빌드 (Windows)
```bat
"<UE>\Engine\Build\BatchFiles\Build.bat" DragonBallSZEditor Win64 Development -Project="<PATH>\DragonBallSZ.uproject" -WaitMutex
"<UE>\Engine\Build\BatchFiles\Build.bat" DragonBallSZ Win64 Development -Project="<PATH>\DragonBallSZ.uproject" -WaitMutex

REM (옵션) 프로젝트 파일 생성
"<UE>\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="<PATH>\DragonBallSZ.uproject"



## 5. 일일 업무 일지 자동 생성 규칙 (09:00 기준)

### 목적
- **전날 09:00 ~ 오늘 09:00** 사이의 Git 커밋을 분석하여 일일 업무 일지를 자동 생성한다.
- 일지 파일 경로: `Documents/DevLog/YYYY-MM-DD.md`
- 에이전트(코덱스)는 **구동할 때마다** 해당 날짜 파일이 없으면 **분석 → 생성**을 수행한다.

### 동작 조건
- 타임존: **Asia/Seoul (KST, UTC+09:00)** 기준으로 09:00 경계를 사용한다.
- 파일 존재 여부:
  - `Documents/DevLog/<TODAY>.md` 가 **없으면** → 커밋 분석 후 신규 생성
  - **있으면** → 건너뜀(중복 생성 금지)

### 커밋 수집 범위
- `START`: **어제 09:00**
- `END`: **오늘 09:00**
- 범위 내 커밋만 분석(기본: `--no-merges`)

### 생성 방법
- Windows: `Tools/RunDevLog.cmd` 실행(아래 스크립트 참조)
- Bash(macOS/Linux/Git Bash) 예시:
  ```bash
  TZ=Asia/Seoul
  TODAY=$(date +%Y-%m-%d)
  START=$(date -d "yesterday 09:00" --iso-8601=seconds)
  END=$(date -d "today 09:00" --iso-8601=seconds)

  OUT="Documents/DevLog/${TODAY}.md"
  if [ ! -f "$OUT" ]; then
    git log --no-merges --since="$START" --until="$END" --date=iso-strict \
      --pretty=format:"%H|%ad|%an|%s%n%b" --numstat > /tmp/daily_commits.txt
    # /tmp/daily_commits.txt를 파싱해 $OUT 생성 (형식은 아래 템플릿과 동일)
  fi
  
  
### Daily DevLog — {{YYYY-MM-DD}} (KST 09:00 경계)

### 요약
- 전날 09:00 ~ 오늘 09:00 사이 커밋 분석 요약 2~3줄.

### 하이라이트
- 영향도 높은 변경 2~5개.

### 커밋 기반 작업 로그
#### 완료
- [scope] subject (SHA7) (+adds/-dels, n files)
#### 진행
- [scope] subject (SHA7) (+adds/-dels, n files)
#### 미흡(주의/후속)
- [scope] subject — 후속: … (SHA7)
#### TODO(커밋 본문 추출)
- [ ] 액션 아이템 (출처: SHA7)

### 오늘 계획
- [ ] 핵심 작업 1
- [ ] 핵심 작업 2
- [ ] 리스크 대응/검증

### 메트릭(선택)
- 총합: +{{addsTotal}} / -{{delsTotal}}, 파일수: {{filesTotal}} 


## 7. 30-Day Briefing — {{YYYY-MM-DD}}

### 속도(Velocity)
- 일 평균 커밋: {{avgCommitsPerDay}} | 완료 비율: {{doneRatio}}%
- 변경 합계: +{{addsTotal}} / -{{delsTotal}}, 파일: {{filesTotal}}

### 핫스팟(Top Scopes)
1) [scope] n건 — 주요 주제 요약
2) [scope] n건 — …

### 미해결 TODO Top N
- [ ] 내용 (출처: 2025-09-xx, SHA7)
- [ ] 내용 (출처: 2025-09-xx, SHA7)

### 리스크/차단요인
- 항목 — 근거/영향/완화책(요약)

### 다음 우선순위(가이드)
1) 액션 — 근거(데이터 포인트)
2) 액션 — 근거
3) 액션 — 근거
---