# YiSan 멀티플레이 로비 시스템 설계 문서

**작성일:** 2025-10-20
**버전:** 1.0
**작성자:** Claude Code
**프로젝트:** YiSan (Unreal Engine 5)

---

## 📑 목차

1. [개요](#1-개요)
2. [시스템 아키텍처](#2-시스템-아키텍처)
3. [핵심 설계 원칙](#3-핵심-설계-원칙)
4. [클래스 다이어그램](#4-클래스-다이어그램)
5. [데이터 흐름](#5-데이터-흐름)
6. [맵 구조 및 전환](#6-맵-구조-및-전환)
7. [난입 허용 메커니즘](#7-난입-허용-메커니즘)
8. [네트워크 복제 전략](#8-네트워크-복제-전략)
9. [주요 기능 명세](#9-주요-기능-명세)
10. [성능 및 확장성](#10-성능-및-확장성)

---

## 1. 개요

### 1.1 목적

YiSan 프로젝트에 멀티플레이 기능을 추가하여 여러 플레이어가 함께 문화유산 투어를 경험할 수 있도록 합니다.

### 1.2 주요 요구사항

- **로비 시스템**: 플레이어가 Host 또는 Join을 선택하는 진입점
- **세션 생성**: Host가 리슨 서버 방식으로 게임 세션 생성
- **세션 참가**: Client가 IP 주소 입력으로 세션 접속
- **난입 허용**: 게임 진행 중에도 새로운 플레이어 합류 가능
- **상태 동기화**: 투어 NPC(Dasan), 퀘스트, 플레이어 상태 복제

### 1.3 기술 스택

- **엔진**: Unreal Engine 5.3+
- **언어**: C++17
- **네트워크**: 리슨 서버 (Listen Server)
- **입력 시스템**: Enhanced Input System
- **UI**: UMG (Unreal Motion Graphics)
- **네트워크 방식**: IP 직접 입력 (OnlineSubsystem 미사용)

---

## 2. 시스템 아키텍처

### 2.1 전체 구조 개요

```
┌─────────────────────────────────────────────────────────────────┐
│                      UYiSanGameInstance                          │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ + HostGame(FString MapName)                               │  │
│  │ + JoinGame(FString IPAddress)                             │  │
│  │ + DisconnectFromSession()                                 │  │
│  │ # ServerTravel(FString MapName)                           │  │
│  │ # ClientTravel(FString Address)                           │  │
│  └───────────────────────────────────────────────────────────┘  │
└──────────────────────┬──────────────────────────────────────────┘
                       │
                       │ 소유
                       │
        ┌──────────────┴──────────────┐
        │                             │
        ▼                             ▼
┌─────────────────┐          ┌──────────────────┐
│ ALobbyGameMode  │          │ AYiSanGameMode   │
│  (LobbyMap용)   │          │  (MainMap용)     │
├─────────────────┤          ├──────────────────┤
│ # PostLogin()   │          │ # PostLogin()    │
│ # Logout()      │          │ # Logout()       │
│ # bAllowJoin    │          │ # bAllowJoin     │
│   = true        │          │   = true         │
└─────────────────┘          └──────────────────┘
        │                             │
        └──────────────┬──────────────┘
                       │
                       │ 생성/관리
                       ▼
              ┌─────────────────┐
              │ APlayerControl  │
              │ (PlayerCtrl)    │
              ├─────────────────┤
              │ + HostSession() │◄──── UI에서 호출
              │ + JoinSession() │◄──── UI에서 호출
              └─────────────────┘
                       │
                       │ 소유
                       ▼
              ┌─────────────────┐
              │  ULobbyWidget   │
              │  (UI Widget)    │
              ├─────────────────┤
              │ • Host 버튼     │
              │ • Join 버튼     │
              │ • IP 입력창     │
              │ • 상태 표시     │
              └─────────────────┘
                       ▲
                       │
                       │ 이벤트 통신
                       │
              ┌─────────────────┐
              │UBroadcastManager│
              │  (Event Bus)    │
              ├─────────────────┤
              │OnSessionHost    │
              │OnSessionJoin    │
              │OnSessionError   │
              └─────────────────┘
```

### 2.2 계층 구조

| 계층 | 클래스 | 역할 |
|------|--------|------|
| **Game Instance** | `UYiSanGameInstance` | 세션 생명주기 관리, 레벨 전환 |
| **Game Mode** | `ALobbyGameMode`, `AYiSanGameMode` | 플레이어 로그인/아웃, 게임 규칙 |
| **Player Controller** | `APlayerControl` | UI ↔ 게임로직 브릿지, 입력 처리 |
| **UI Layer** | `ULobbyWidget` | 사용자 인터페이스, 버튼 이벤트 |
| **Event Bus** | `UBroadcastManager` | 전역 이벤트 전파 (Subsystem) |

---

## 3. 핵심 설계 원칙

### 3.1 리슨 서버 방식

- **Dedicated Server 불필요**: Host 플레이어가 서버 역할을 겸함
- **비용 효율적**: 별도 서버 인프라 불필요
- **간단한 구조**: P2P 형태로 빠른 구현 가능

### 3.2 IP 직접 입력 방식

- **OnlineSubsystem 미사용**: 복잡한 세션 브라우징 생략
- **Tailscale MagicDNS 지원**: 도메인 이름으로 접속 가능 (예: `host-pc`)
- **빠른 접속**: IP 입력 후 즉시 연결

### 3.3 난입 허용 (Join In Progress)

- **동적 플레이어 추가**: 게임 진행 중에도 새 플레이어 합류
- **상태 동기화**: 난입 플레이어에게 현재 게임 상태 자동 복제
- **유연한 플레이**: 친구가 언제든지 참여 가능

### 3.4 이벤트 기반 아키텍처

- **UBroadcastManager**: 중앙 집중식 이벤트 버스
- **느슨한 결합**: 클래스 간 직접 의존성 최소화
- **확장성**: 새로운 이벤트 추가 용이

---

## 4. 클래스 다이어그램

### 4.1 UYiSanGameInstance

```cpp
class UYiSanGameInstance : public UGameInstance
{
    // Level Management
    FName TargetLevel;
    bool bLevelReady;

    // Multiplayer Session Management
    + bool HostGame(FString MapName, int32 MaxPlayers);
    + bool JoinGame(FString Address, int32 Port);
    + void DisconnectFromSession();
    + bool IsHost() const;
    + bool IsInSession() const;

    # void ServerTravel(FString MapName);
    # void ClientTravel(FString Address);

    - bool bIsHost;
    - bool bIsInSession;
};
```

**책임:**
- 세션 생성 및 참가 로직
- 레벨 전환 (ServerTravel/ClientTravel)
- 호스트/클라이언트 상태 추적

### 4.2 ALobbyGameMode

```cpp
class ALobbyGameMode : public AGameModeBase
{
    // Overrides
    + virtual void PostLogin(APlayerController* NewPlayer) override;
    + virtual void Logout(AController* Exiting) override;

    // Lobby Settings
    + bool bAllowJoinInProgress = true;
    + int32 MaxPlayers = 4;

    // Lobby State
    + int32 GetCurrentPlayerCount() const;
    + bool IsLobbyFull() const;

    - int32 CurrentPlayerCount;
};
```

**책임:**
- 로비 맵에서 플레이어 입장/퇴장 처리
- 플레이어 수 추적
- Pawn 생성 보장

### 4.3 AYiSanGameMode

```cpp
class AYiSanGameMode : public AGameModeBase
{
    + virtual void PostLogin(APlayerController* NewPlayer) override;
    + void StartTour();
    + void SetTourState(EDasanState InState);

    - TObjectPtr<ADasanActor> DasanNPC;
};
```

**책임:**
- 메인 게임 로직 (투어 시스템)
- 난입 플레이어 처리
- DasanNPC 관리 및 GameState 동기화

### 4.4 APlayerControl

```cpp
class APlayerControl : public APlayerController
{
    // Enhanced Input
    # TObjectPtr<UInputMappingContext> IMC_Default;
    # TObjectPtr<UInputAction> IA_Move, IA_Look, ...;

    // Multiplayer Session Functions (UI Binding)
    + void HostSession(FString MapName, int32 MaxPlayers);
    + void JoinSession(FString Address, int32 Port);
    + void DisconnectSession();

    # void OnMove(const FInputActionValue& Value);
    # void OnLook(const FInputActionValue& Value);
    ...

    - IControllable* GetControllable() const;
    - TObjectPtr<APlayerControl> CachedPlayerController;
};
```

**책임:**
- UI 위젯과 GameInstance 연결
- 입력 처리 (Enhanced Input)
- IControllable 인터페이스를 통한 명령 전달

### 4.5 ULobbyWidget

```cpp
class ULobbyWidget : public UUserWidget
{
    // UI Bindings
    UPROPERTY(meta=(BindWidget))
    + TObjectPtr<UButton> Btn_Host;
    + TObjectPtr<UButton> Btn_Join;
    + TObjectPtr<UButton> Btn_Disconnect;
    + TObjectPtr<UEditableTextBox> TxtBox_IPAddress;
    + TObjectPtr<UTextBlock> Txt_Status;

    // Settings
    + FString MapName = "MainMap_WP";
    + int32 MaxPlayers = 4;
    + int32 Port = 7777;

    // Button Handlers
    + void OnHostButtonClicked();
    + void OnJoinButtonClicked();
    + void OnDisconnectButtonClicked();

    // Broadcast Event Handlers
    + void OnSessionHost(FString MapName);
    + void OnSessionJoin(FString Address, int32 Port);
    + void OnSessionDisconnect();
    + void OnSessionError(FString ErrorMessage);

    // UI Update
    + void UpdateStatusText(FString StatusText);

    - TObjectPtr<APlayerControl> CachedPlayerController;
};
```

**책임:**
- 사용자 입력 수신 (버튼 클릭, IP 입력)
- PlayerController의 세션 함수 호출
- BroadcastManager 이벤트 수신 및 UI 업데이트
- 상태 피드백 표시

### 4.6 UBroadcastManager

```cpp
class UBroadcastManager : public UGameInstanceSubsystem
{
    // Multiplayer Session Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionHost, FString, MapName);
    + FOnSessionHost OnSessionHost;
    + void SendSessionHost(FString MapName);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionJoin, FString, Address, int32, Port);
    + FOnSessionJoin OnSessionJoin;
    + void SendSessionJoin(FString Address, int32 Port);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionDisconnect);
    + FOnSessionDisconnect OnSessionDisconnect;
    + void SendSessionDisconnect();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionError, FString, ErrorMessage);
    + FOnSessionError OnSessionError;
    + void SendSessionError(FString ErrorMessage);
};
```

**책임:**
- 멀티플레이 세션 이벤트 브로드캐스트
- 전역 이벤트 버스 역할
- 느슨한 결합을 통한 클래스 간 통신

---

## 5. 데이터 흐름

### 5.1 Host 세션 생성 플로우

```
1. ULobbyWidget::OnHostButtonClicked()
   │
   ├─ MapName = "MainMap_WP"
   ├─ MaxPlayers = 4
   │
   ▼
2. APlayerControl::HostSession(MapName, MaxPlayers)
   │
   ├─ GetGameInstance() → Cast to UYiSanGameInstance
   │
   ▼
3. UYiSanGameInstance::HostGame(MapName, MaxPlayers)
   │
   ├─ TravelURL = "/Game/YiSan/Maps/MainMap_WP?listen?MaxPlayers=4"
   ├─ World->ServerTravel(TravelURL)
   ├─ bIsHost = true
   ├─ bIsInSession = true
   │
   ▼
4. UBroadcastManager::SendSessionHost(MapName)
   │
   ├─ OnSessionHost.Broadcast(MapName)
   │
   ▼
5. ULobbyWidget::OnSessionHost(MapName)
   │
   ├─ UpdateStatusText("호스트 생성 완료!")
   ├─ Btn_Host->SetVisibility(Collapsed)
   ├─ Btn_Join->SetVisibility(Collapsed)
   └─ Btn_Disconnect->SetVisibility(Visible)
```

### 5.2 Client 세션 참가 플로우

```
1. ULobbyWidget::OnJoinButtonClicked()
   │
   ├─ IPAddress = TxtBox_IPAddress->GetText() // "192.168.0.10"
   ├─ Port = 7777
   │
   ▼
2. APlayerControl::JoinSession(IPAddress, Port)
   │
   ├─ GetGameInstance() → Cast to UYiSanGameInstance
   │
   ▼
3. UYiSanGameInstance::JoinGame(IPAddress, Port)
   │
   ├─ TravelURL = "192.168.0.10:7777"
   ├─ GetFirstLocalPlayerController()->ClientTravel(TravelURL, TRAVEL_Absolute)
   ├─ bIsHost = false
   ├─ bIsInSession = true
   │
   ▼
4. UBroadcastManager::SendSessionJoin(IPAddress, Port)
   │
   ├─ OnSessionJoin.Broadcast(IPAddress, Port)
   │
   ▼
5. ULobbyWidget::OnSessionJoin(IPAddress, Port)
   │
   ├─ UpdateStatusText("서버 접속 완료!")
   ├─ Btn_Host->SetVisibility(Collapsed)
   ├─ Btn_Join->SetVisibility(Collapsed)
   └─ Btn_Disconnect->SetVisibility(Visible)
```

### 5.3 세션 연결 해제 플로우

```
1. ULobbyWidget::OnDisconnectButtonClicked()
   │
   ▼
2. APlayerControl::DisconnectSession()
   │
   ▼
3. UYiSanGameInstance::DisconnectFromSession()
   │
   ├─ UGameplayStatics::OpenLevel(this, "LobbyMap", false)
   ├─ bIsHost = false
   ├─ bIsInSession = false
   │
   ▼
4. UBroadcastManager::SendSessionDisconnect()
   │
   ▼
5. ULobbyWidget::OnSessionDisconnect()
   │
   ├─ UpdateStatusText("연결 해제됨")
   ├─ Btn_Disconnect->SetVisibility(Collapsed)
   ├─ Btn_Host->SetVisibility(Visible)
   └─ Btn_Join->SetVisibility(Visible)
```

---

## 6. 맵 구조 및 전환

### 6.1 맵 계층

```
게임 시작
   │
   ▼
┌──────────────┐
│  LobbyMap    │  ← ALobbyGameMode 적용
│              │  ← 플레이어 선택 화면
│ [Host 버튼]  │
│ [Join 버튼]  │
│ [IP 입력]    │
└──────┬───────┘
       │
       ├─ Host 선택: ServerTravel("/Game/YiSan/Maps/MainMap_WP?listen?MaxPlayers=4")
       │
       └─ Join 선택: ClientTravel("192.168.0.10:7777")
       │
       ▼
┌──────────────┐
│ MainMap_WP   │  ← AYiSanGameMode 적용
│              │  ← 리슨 서버 (난입 허용)
│ [게임플레이] │  ← 투어 시스템 활성화
│ [DasanNPC]   │  ← 문화유산 가이드
│              │
└──────────────┘
       ▲
       │
       └─ 추가 플레이어 난입: ClientTravel("192.168.0.10:7777")
          (게임 진행 중에도 가능)
```

### 6.2 맵별 GameMode 설정

| 맵 이름 | GameMode | 용도 | 특징 |
|---------|----------|------|------|
| **LobbyMap** | `ALobbyGameMode` | 세션 선택 화면 | 단순 UI 표시, 플레이어 대기 |
| **MainMap_WP** | `AYiSanGameMode` | 메인 게임플레이 | 투어 시스템, NPC 상호작용, 난입 허용 |

### 6.3 레벨 전환 방식

#### ServerTravel (Host)

```cpp
// UYiSanGameInstance::ServerTravel()
void UYiSanGameInstance::ServerTravel(const FString& MapName)
{
    if (UWorld* World = GetWorld())
    {
        World->ServerTravel(MapName); // 모든 연결된 클라이언트도 함께 이동
    }
}
```

- **용도**: Host가 새 맵으로 이동할 때
- **특징**: 모든 클라이언트가 함께 이동
- **옵션**: `?listen` - 리슨 서버 활성화

#### ClientTravel (Client)

```cpp
// UYiSanGameInstance::ClientTravel()
void UYiSanGameInstance::ClientTravel(const FString& Address)
{
    if (APlayerController* PC = GetFirstLocalPlayerController())
    {
        PC->ClientTravel(Address, TRAVEL_Absolute);
    }
}
```

- **용도**: Client가 서버에 접속할 때
- **특징**: 해당 클라이언트만 이동
- **포맷**: `IP:Port` (예: `192.168.0.10:7777`)

---

## 7. 난입 허용 메커니즘

### 7.1 리슨 서버 구조

```cpp
// Host 플레이어가 리슨 서버 생성
World->ServerTravel("/Game/YiSan/Maps/MainMap_WP?listen?MaxPlayers=4");
```

- **`?listen` 옵션**: 서버 소켓을 열어 클라이언트 접속 대기
- **포트**: 기본값 7777 (DefaultEngine.ini에서 설정 가능)
- **동시 처리**: Host가 게임 플레이 + 서버 역할 동시 수행

### 7.2 난입 시나리오

```
[시나리오: 게임 진행 중 난입]

T=0s  : Host가 MainMap_WP에서 투어 시작
        ├─ DasanNPC가 Yeomingak 건물 설명 중
        └─ Listen Server 활성화 (포트 7777 OPEN)

T=60s : 새 플레이어(Client)가 LobbyMap에서 Join 버튼 클릭
        ├─ IP 입력: "192.168.0.10"
        └─ ClientTravel("192.168.0.10:7777") 실행

T=61s : 서버가 새 플레이어 연결 수신
        ├─ AYiSanGameMode::PostLogin(NewPlayer) 호출
        ├─ PlayerController 생성
        ├─ APawn 생성 (SpawnDefaultPawnFor)
        ├─ NewPlayer->Possess(Pawn)
        └─ GameState 복제 시작

T=62s : 난입 플레이어가 현재 게임 상태 수신
        ├─ DasanNPC 위치 및 상태 (Replicated)
        ├─ GlobalTourState = Explain (Replicated)
        ├─ bIsTourActive = true (Replicated)
        ├─ 기존 플레이어들의 위치/상태
        └─ 건물 데이터 동기화

T=63s : 난입 플레이어가 정상적으로 게임 참여
        └─ Host 플레이어와 함께 투어 진행
```

### 7.3 PostLogin 처리

```cpp
// AYiSanGameMode::PostLogin()
void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    PRINTLOG(TEXT("[GameMode] PostLogin - PC=%s Pawn=%s"),
        *GetNameSafe(NewPlayer),
        *GetNameSafe(NewPlayer->GetPawn()));

    // Pawn이 없으면 강제로 생성 (난입 플레이어 보호)
    if (NewPlayer && !NewPlayer->GetPawn())
    {
        AActor* PlayerStart = FindPlayerStart(NewPlayer);
        APawn* NewPawn = SpawnDefaultPawnFor(NewPlayer, PlayerStart);

        if (NewPawn)
        {
            NewPlayer->Possess(NewPawn);
            PRINTLOG(TEXT("[GameMode] Forced possess: %s"), *GetNameSafe(NewPawn));
        }
    }

    // DasanNPC 찾기 및 GameState 설정
    if (DasanNPC == nullptr)
    {
        auto FoundActors = FComponentHelper::GetAllOfClass<ADasanActor>(GetWorld());
        if (FoundActors.Num() > 0)
        {
            DasanNPC = FoundActors[0];
            // GameState에 설정하면 자동으로 복제됨
            if (auto State = GetGameState<AYisanGameState>())
            {
                State->DasanNPC = DasanNPC; // Replicated Property
            }
        }
    }
}
```

**핵심 포인트:**
1. **Pawn 보장**: 난입 플레이어에게 반드시 Pawn 생성
2. **GameState 동기화**: DasanNPC 등 공유 상태를 GameState에 설정
3. **자동 복제**: Replicated 속성은 자동으로 난입 플레이어에게 전송

---

## 8. 네트워크 복제 전략

### 8.1 복제 대상

| 클래스 | 속성 | 복제 방식 | 용도 |
|--------|------|-----------|------|
| `AYisanGameState` | `DasanNPC` | `Replicated` | 투어 가이드 NPC 참조 |
| `AYisanGameState` | `GlobalTourState` | `Replicated` | 전역 투어 상태 (Tour/Explain/Answer) |
| `AYisanGameState` | `bIsTourActive` | `Replicated` | 투어 활성화 여부 |
| `ADasanActor` | `DasanState` | `ReplicatedUsing=OnRep_DasanState` | NPC 상태 변경 시 OnRep 호출 |

### 8.2 GameState 복제 구조

```cpp
// AYisanGameState.h (기존 코드)
UCLASS()
class AYisanGameState : public AGameStateBase
{
    GENERATED_BODY()

    // 복제될 속성들
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
    TObjectPtr<ADasanActor> DasanNPC;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
    EDasanState GlobalTourState;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
    bool bIsTourActive;

    // 복제 설정
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
    {
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);

        DOREPLIFETIME(AYisanGameState, DasanNPC);
        DOREPLIFETIME(AYisanGameState, GlobalTourState);
        DOREPLIFETIME(AYisanGameState, bIsTourActive);
    }
};
```

### 8.3 NPC 상태 복제

```cpp
// ADasanActor.h (기존 코드)
UCLASS()
class ADasanActor : public AActor
{
    GENERATED_BODY()

    // 상태 변경 시 OnRep 호출
    UPROPERTY(ReplicatedUsing=OnRep_DasanState, BlueprintReadOnly, Category="State")
    EDasanState DasanState;

    // 서버 RPC (클라이언트에서 호출 → 서버에서 실행)
    UFUNCTION(Server, Reliable)
    void ServerRPC_SetDasanState(EDasanState InState);

    // 복제 콜백 (서버 → 클라이언트 전송 완료 시 호출)
    UFUNCTION()
    void OnRep_DasanState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
    {
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);
        DOREPLIFETIME(ADasanActor, DasanState);
    }
};
```

### 8.4 난입 플레이어 동기화 과정

```
1. 난입 플레이어 접속
   │
   ▼
2. 서버가 GameState 복제 시작
   ├─ AYisanGameState::DasanNPC → Client로 전송
   ├─ AYisanGameState::GlobalTourState → Client로 전송
   └─ AYisanGameState::bIsTourActive → Client로 전송
   │
   ▼
3. DasanNPC Actor 복제
   ├─ ADasanActor::DasanState → Client로 전송
   ├─ Transform (위치, 회전) → Client로 전송
   └─ 애니메이션 상태 → Client로 전송
   │
   ▼
4. 클라이언트에서 OnRep 콜백 실행
   ├─ ADasanActor::OnRep_DasanState() 호출
   └─ UI 업데이트, 애니메이션 재생 등
   │
   ▼
5. 동기화 완료
   └─ 난입 플레이어가 현재 상태를 볼 수 있음
```

---

## 9. 주요 기능 명세

### 9.1 세션 생성 (Host)

**입력:**
- `MapName`: 호스팅할 맵 이름 (기본값: "MainMap_WP")
- `MaxPlayers`: 최대 플레이어 수 (기본값: 4)

**출력:**
- `bool`: 세션 생성 성공 여부

**동작:**
1. TravelURL 생성: `/Game/YiSan/Maps/{MapName}?listen?MaxPlayers={MaxPlayers}`
2. `World->ServerTravel(TravelURL)` 호출
3. 리슨 서버 활성화 (포트 7777)
4. `bIsHost = true`, `bIsInSession = true` 설정
5. `OnSessionHost` 이벤트 브로드캐스트

### 9.2 세션 참가 (Join)

**입력:**
- `Address`: 서버 IP 주소 또는 도메인 (예: "192.168.0.10", "host-pc")
- `Port`: 서버 포트 (기본값: 7777)

**출력:**
- `bool`: 세션 참가 시도 성공 여부

**동작:**
1. TravelURL 생성: `{Address}:{Port}`
2. `PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute)` 호출
3. 서버 접속 시도
4. `bIsHost = false`, `bIsInSession = true` 설정
5. `OnSessionJoin` 이벤트 브로드캐스트

### 9.3 세션 연결 해제

**입력:** 없음

**출력:** 없음

**동작:**
1. `UGameplayStatics::OpenLevel(this, "LobbyMap", false)` 호출
2. 로비 맵으로 복귀
3. `bIsHost = false`, `bIsInSession = false` 초기화
4. `OnSessionDisconnect` 이벤트 브로드캐스트

### 9.4 난입 허용

**조건:**
- `ALobbyGameMode::bAllowJoinInProgress = true`
- `AYiSanGameMode::bAllowJoinInProgress = true` (상속 가능)
- 리슨 서버 활성화 상태

**동작:**
1. 새 클라이언트가 `ClientTravel(IP:Port)` 실행
2. 서버가 연결 수락
3. `PostLogin()` 호출하여 Pawn 생성
4. GameState 및 복제된 Actor 동기화
5. 플레이어가 게임에 참여

---

## 10. 성능 및 확장성

### 10.1 네트워크 대역폭

| 항목 | 권장 설정 | 설명 |
|------|-----------|------|
| **NetServerMaxTickRate** | 120 | 서버 틱 레이트 |
| **MaxNetTickRate** | 120 | 클라이언트 틱 레이트 |
| **MaxInternetClientRate** | 100000 | 인터넷 클라이언트 최대 전송률 (bytes/s) |
| **MaxClientRate** | 100000 | LAN 클라이언트 최대 전송률 (bytes/s) |

### 10.2 복제 최적화

- **조건부 복제**: `DOREPLIFETIME_CONDITION` 사용하여 필요한 클라이언트에만 복제
- **복제 빈도 조절**: `NetUpdateFrequency` 설정으로 업데이트 빈도 최적화
- **Relevancy**: 거리 기반 복제로 불필요한 Actor 복제 방지

### 10.3 확장 가능성

#### 미래 확장 항목

1. **세션 브라우징**: OnlineSubsystem 통합으로 세션 목록 표시
2. **전용 서버**: Dedicated Server 지원 추가
3. **매치메이킹**: 자동 매칭 시스템 구현
4. **보이스챗**: VoIP 통합 (이미 UVoiceConversationSystem 존재)
5. **권한 시스템**: Host만 투어 시작 가능 등의 권한 설정

#### 모듈화 구조

현재 설계는 각 기능이 독립적으로 동작하므로 확장이 용이합니다:
- `UBroadcastManager`: 새 이벤트 추가 가능
- `UYiSanGameInstance`: 새 네트워크 기능 추가 가능
- `ULobbyWidget`: UI 요소 추가 가능

---

## 부록 A: 파일 구조

### A.1 수정된 파일

```
YiSan/
├─ Source/
│  ├─ YiSan/
│  │  ├─ Loading/
│  │  │  ├─ Public/
│  │  │  │  └─ YiSanGameInstance.h           [수정]
│  │  │  └─ Private/
│  │  │     └─ YiSanGameInstance.cpp          [수정]
│  │  ├─ Environment/
│  │  │  ├─ Public/
│  │  │  │  ├─ ALobbyGameMode.h               [생성]
│  │  │  │  └─ AYiSanGameMode.h               [기존]
│  │  │  └─ Private/
│  │  │     ├─ ALobbyGameMode.cpp             [생성]
│  │  │     └─ AYiSanGameMode.cpp             [기존]
│  │  └─ UI/
│  │     ├─ Public/
│  │     │  └─ ULobbyWidget.h                 [생성]
│  │     └─ Private/
│  │        └─ ULobbyWidget.cpp                [생성]
│  └─ LatteLibrary/
│     ├─ Character/
│     │  ├─ Public/
│     │  │  └─ APlayerControl.h               [수정]
│     │  └─ Private/
│     │     └─ APlayerControl.cpp             [수정]
│     └─ Manager/
│        ├─ Public/
│        │  └─ UBroadcastManager.h            [수정]
│        └─ Private/
│           └─ UBroadcastManager.cpp          [수정]
```

### A.2 Blueprint 파일 (생성 필요)

```
Content/
├─ YiSan/
│  ├─ Maps/
│  │  ├─ LobbyMap.umap                         [생성]
│  │  └─ MainMap_WP.umap                       [기존]
│  └─ UI/
│     └─ WBP_LobbyWidget.uasset                [생성]
```

---

## 부록 B: 참고 자료

### B.1 언리얼 엔진 문서

- [Multiplayer Programming Quick Start](https://docs.unrealengine.com/5.3/en-US/multiplayer-programming-quick-start-for-unreal-engine/)
- [Networking Overview](https://docs.unrealengine.com/5.3/en-US/networking-overview-for-unreal-engine/)
- [Actor Replication](https://docs.unrealengine.com/5.3/en-US/actor-replication-in-unreal-engine/)
- [GameInstance](https://docs.unrealengine.com/5.3/en-US/API/Runtime/Engine/Engine/UGameInstance/)

### B.2 프로젝트 규칙

- **코딩 스타일**: `CLAUDE.md`, `AgentRule/PERSONA.md` 참조
- **Doxygen 주석**: 모든 public 함수에 주석 필수
- **로깅**: `PRINTLOG` 매크로 사용 (`GameLogging.h`)
- **매크로**: `Macro.h`의 `DEFINE_SUBSYSTEM_GETTER_INLINE` 활용

---

**문서 끝**
