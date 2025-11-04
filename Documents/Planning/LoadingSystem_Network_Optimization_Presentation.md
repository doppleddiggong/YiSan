# YiSan 로딩 시스템: 네트워크 최적화 관점 발표

## 1. 시스템 개요

### 핵심 컴포넌트
```
┌─────────────────────────────────────────────────────────────┐
│                    네트워크 로딩 아키텍처                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  [Server]                    [Network]            [Client]   │
│                                                               │
│  UYiSanLoading    ──RPC──>   Replication   ──>  PlayerControl│
│  (GameInstance)              Layer               (각 클라이언트)│
│       │                                               │       │
│       │ ServerTravel                    ClientRPC_Show│       │
│       │                                 Loading       │       │
│       └─────────────────────────────────────────────>│       │
│                                                       │       │
│                                                ULoadingTransition│
│                                                Manager        │
│                                              (LocalPlayer     │
│                                               Subsystem)      │
└─────────────────────────────────────────────────────────────┘
```

### 주요 클래스
- **UYiSanLoading**: GameInstanceSubsystem - 서버 측 로딩 오케스트레이션
- **ULoadingTransitionManager**: LocalPlayerSubsystem - 클라이언트 측 UI 관리
- **APlayerControl**: PlayerController - 네트워크 통신 브리지

---

## 2. 네트워크 플로우 분석

### 2.1 서버 측 시퀀스
```
InitSystem(URL, bAbsolute, bUseLoadingScreen)
    ↓
PrepareForTravel()
    ↓
Broadcast_ShowLoading()  ─────RPC────> 모든 클라이언트
    ↓
World->ServerTravel(URL)
    ↓
PostLoadMapWithWorld()
    ↓
UpdateTick() - 3단계 로딩 체크
    │
    ├─> [1] WorldPartition 스트리밍
    ├─> [2] Texture 스트리밍
    └─> [3] LevelInstance 로딩
    ↓
CompleteProcess()
    ↓
Broadcast_HideLoading()  ─────RPC────> 모든 클라이언트
```

### 2.2 클라이언트 측 시퀀스
```
[클라이언트 수신]

ClientRPC_ShowLoadingTransition()
    ↓
ShowLoadingScreenLocal()
    ↓
ULoadingTransitionManager::ShowLoadingScreen()
    ↓
[로딩 UI 표시, 입력 비활성화]

... 서버 로딩 진행 중 ...

ClientRPC_HideLoadingTransition()
    ↓
HandleLoadingComplete()
    ↓
IsReadyToFinish() 체크
    │
    ├─> Pawn 초기화 확인
    └─> 렌더링 준비 대기 (3초)
    ↓
CompleteLoading()
    ↓
ULoadingTransitionManager::HideLoadingScreen()
    ↓
[로딩 UI 숨김, 입력 활성화]
```

---

## 3. 네트워크 최적화 포인트

### 3.1 RPC 최적화
**현재 구조:**
```cpp
// UYiSanLoading.cpp:380
void UYiSanLoading::Broadcast_ShowLoading() const
{
    if (World->GetNetMode() == NM_Client)
    {
        // 클라이언트 모드: 로컬 처리만
        LocalPC->ShowLoadingScreenLocal();
        return;
    }

    // 서버 모드: 모든 연결된 클라이언트에게 브로드캐스트
    for (auto It = World->GetPlayerControllerIterator(); It; ++It)
    {
        pc->ClientRPC_ShowLoadingTransition();
    }
}
```

**최적화 효과:**
- ✅ **불필요한 RPC 제거**: 클라이언트 모드에서는 RPC 없이 로컬 처리
- ✅ **네트워크 트래픽 감소**: 자기 자신에게 RPC 전송 방지
- ✅ **레이턴시 개선**: 로컬 작업은 즉시 실행

### 3.2 상태 동기화 전략
**서버 주도형 로딩:**
```cpp
// 서버가 모든 로딩 상태를 관리
enum struct EState : uint8
{
    WP          // WorldPartition 스트리밍
    TEXTURE     // 텍스처 스트리밍
    LI          // LevelInstance 로딩
    COMPLETE    // 완료
};

// 클라이언트는 서버의 완료 신호만 대기
// → 네트워크 동기화 복잡도 최소화
```

**장점:**
- ✅ **단일 진실 소스 (Single Source of Truth)**: 서버가 권위 있는 상태 관리
- ✅ **동기화 오버헤드 최소화**: 시작/종료 신호만 RPC로 전송
- ✅ **클라이언트 부담 감소**: 복잡한 로딩 로직은 서버에서만 실행

### 3.3 타이밍 최적화
**비동기 완료 처리:**
```cpp
// APlayerControl.cpp:435
void APlayerControl::HandleLoadingComplete()
{
    if (!IsReadyToFinish())
    {
        bAwaitFinish = true;  // 완료 대기 플래그 설정
        return;  // 준비될 때까지 대기
    }

    CompleteLoading();
}

// APlayerControl.cpp:473 - 렌더링 준비 추가 대기
FTimerHandle RenderWaitHandle;
GetWorld()->GetTimerManager().SetTimer(
    RenderWaitHandle,
    [this]() { /* 실제 로딩 화면 숨김 */ },
    3.0f,  // 렌더링 준비 시간
    false
);
```

**최적화 효과:**
- ✅ **시각적 완성도**: 렌더링 준비 완료 후 화면 전환
- ✅ **RPC 중복 방지**: bAwaitFinish 플래그로 중복 호출 차단
- ✅ **사용자 경험 개선**: 블랙 화면 노출 최소화

---

## 4. 로딩 단계별 네트워크 영향

### 4.1 WorldPartition 스트리밍
```cpp
// UYiSanLoading.cpp:177
if (auto WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
    CompleteState[EState::WP] = WPS->IsStreamingCompleted();
```

**네트워크 특성:**
- 서버/클라이언트 모두 **독립적으로** 스트리밍
- 네트워크 트래픽: **높음** (Actor 복제, 트랜스폼 동기화)
- 최적화: Replication Graph를 통한 관련성 필터링

### 4.2 Texture 스트리밍
```cpp
// UYiSanLoading.cpp:274
void UYiSanLoading::Loading_Textures(const UWorld* InWorld)
{
    IStreamingManager& StreamingManager = IStreamingManager::Get();
    const int32 PendingRequests = StreamingManager.GetNumWantingResources();

    // 진행률 계산
    Progress_Texture = 1.0f - (PendingRequests / TextureRequestCount);

    // 타임아웃 처리 (60초)
    if (ElapsedTime > TextureStreaming_TimeOut)
        CompleteState[EState::TEXTURE] = true;
}
```

**네트워크 특성:**
- 네트워크 트래픽: **없음** (로컬 리소스 로딩)
- 각 클라이언트가 **독립적**으로 텍스처 스트리밍
- 타임아웃으로 네트워크 대기 방지

### 4.3 LevelInstance 로딩
```cpp
// UYiSanLoading.cpp:335
void UYiSanLoading::Loading_LevelInstance(UWorld* InWorld)
{
    auto Found = FComponentHelper::GetAllOfClass<ALevelInstance>(InWorld);

    int32 ReadyCount = 0;
    for (auto LevelInstance : Found)
    {
        if (LevelInstance && LevelInstance->GetLoadedLevel() != nullptr)
            ++ReadyCount;
    }

    Progress_LI = ReadyCount / Found.Num();
}
```

**네트워크 특성:**
- 네트워크 트래픽: **중간** (레벨 내 Actor 복제)
- 서브레벨 로딩 후 Actor 동기화 필요
- 진행률 추적으로 사용자 피드백 제공

---

## 5. 핵심 최적화 전략

### 5.1 Seamless Travel 활용
```cpp
// ServerTravel 호출 시 자동으로 Seamless Travel 적용
World->ServerTravel(InURL, bAbsolute);

// Engine이 자동으로:
// 1. 필수 Actor 유지 (PlayerController, GameState 등)
// 2. 레벨 전환 중 연결 유지
// 3. 로딩 화면 표시
```

**네트워크 이점:**
- ✅ **연결 유지**: 클라이언트 재연결 불필요
- ✅ **데이터 보존**: PlayerState, GameState 유지
- ✅ **대역폭 절약**: 기존 연결 재사용

### 5.2 LocalPlayerSubsystem 활용
```cpp
// ULoadingTransitionManager.h:11
class ULoadingTransitionManager : public ULocalPlayerSubsystem
```

**네트워크 이점:**
- ✅ **클라이언트 측 전용**: 서버 리소스 절약
- ✅ **로컬 상태 관리**: UI 상태 복제 불필요
- ✅ **멀티플레이어 지원**: 각 LocalPlayer마다 독립적인 인스턴스

### 5.3 델리게이트 기반 이벤트 처리
```cpp
// ULoadingTransitionManager.cpp:31
PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(
    this, &ULoadingTransitionManager::HandlePreLoadMap);

PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
    this, &ULoadingTransitionManager::HandlePostLoadMap);
```

**네트워크 이점:**
- ✅ **자동 동기화**: 엔진 이벤트에 자동 반응
- ✅ **폴링 제거**: 주기적인 상태 체크 불필요
- ✅ **리소스 효율**: 이벤트 기반 처리

---

## 6. 성능 메트릭

### 6.1 RPC 호출 횟수
**플레이어 N명 기준:**
- ShowLoading RPC: **N회** (서버 → 각 클라이언트)
- HideLoading RPC: **N회** (서버 → 각 클라이언트)
- **총 2N회 RPC** (최소화된 통신)

### 6.2 네트워크 대역폭
```
┌──────────────────────────────────────────────────┐
│ 로딩 단계별 네트워크 사용량                        │
├──────────────────────────────────────────────────┤
│ 1. RPC 전송        : ~1-2 KB (압축 후)           │
│ 2. WorldPartition  : 수백 KB ~ 수 MB (Actor 복제)│
│ 3. Texture         : 0 KB (로컬 로딩)            │
│ 4. LevelInstance   : 수십 KB ~ 수백 KB           │
├──────────────────────────────────────────────────┤
│ 총 네트워크 사용량: 주로 Actor 복제에 집중        │
│ RPC 오버헤드: 전체의 < 1%                        │
└──────────────────────────────────────────────────┘
```

### 6.3 타이밍 메트릭
```cpp
// UYiSanLoading.cpp:234
PRINTLOG(TEXT("총 소요 시간: %.2f초"), FPlatformTime::Seconds() - TotalTime);

// 일반적인 로딩 시간 분포:
// - WorldPartition    : 1-3초
// - Texture Streaming : 2-10초
// - LevelInstance     : 1-5초
// - 렌더링 준비       : 3초 (고정)
// ────────────────────────────
// 총 예상 시간        : 7-21초
```

---

## 7. 코드 레퍼런스

### 주요 파일 위치
```
Source/YiSan/
├── Loading/
│   ├── Public/UYiSanLoading.h          (서버 로딩 시스템)
│   └── Private/UYiSanLoading.cpp       (로딩 로직 구현)
│
├── UI/
│   ├── Public/ULoadingTransitionManager.h   (클라이언트 UI 관리자)
│   ├── Private/ULoadingTransitionManager.cpp
│   ├── Public/ULoadingTransitionWidget.h    (로딩 위젯)
│   └── Private/ULoadingTransitionWidget.cpp
│
└── Character/
    ├── Public/APlayerControl.h         (네트워크 브리지)
    └── Private/APlayerControl.cpp      (RPC 구현)
```

### 핵심 함수 위치
```cpp
// 서버 측 진입점
UYiSanLoading::InitSystem()                    // Line 25

// RPC 브로드캐스트
UYiSanLoading::Broadcast_ShowLoading()         // Line 366
UYiSanLoading::Broadcast_HideLoading()         // Line 390

// 클라이언트 RPC 수신
APlayerControl::ClientRPC_ShowLoadingTransition_Implementation()  // Line 423
APlayerControl::ClientRPC_HideLoadingTransition_Implementation()  // Line 429

// 클라이언트 완료 처리
APlayerControl::HandleLoadingComplete()        // Line 435
APlayerControl::CompleteLoading()              // Line 468

// UI 관리
ULoadingTransitionManager::ShowLoadingScreen() // Line 112
ULoadingTransitionManager::HideLoadingScreen() // Line 148
```

---

## 8. 결론

### 네트워크 최적화 핵심 원칙
1. **최소 RPC 전략**: 시작/종료 신호만 네트워크 전송
2. **서버 권위 모델**: 로딩 상태는 서버가 단일 관리
3. **로컬 우선 처리**: UI 및 리소스 로딩은 클라이언트 독립 실행
4. **비동기 완료**: 각 클라이언트가 준비 완료 시점 자체 결정

### 시스템 장점
✅ **확장성**: 플레이어 수 증가에도 RPC 오버헤드 선형 증가
✅ **안정성**: 타임아웃 처리로 무한 대기 방지
✅ **사용자 경험**: 시각적 완성도 보장 후 전환
✅ **디버깅 용이**: 단계별 로그로 문제 추적 가능

### 추가 개선 가능 영역
- 🔄 **프리페칭**: 다음 맵 예상 및 미리 로딩
- 📊 **동적 타임아웃**: 네트워크 상태 기반 타임아웃 조정
- 🎮 **점진적 활성화**: 로딩 완료된 영역부터 순차적 공개
- 📈 **진행률 동기화**: 서버가 전체 진행률을 클라이언트에 브로드캐스트

---

**발표 준비 완료**
이 자료는 YiSan 프로젝트의 LoadingTransition 및 YisanLoading 시스템의 네트워크 최적화 구조를 상세히 설명합니다.
