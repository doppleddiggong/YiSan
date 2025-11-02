# PlayerIndex 관리 시스템 완전 해결 보고서

**프로젝트:** YiSan
**작성일:** 2025-11-02
**상태:** ✅ 완전 해결
**문서 버전:** 1.0 Final

---

## 📋 목차

1. [개요](#개요)
2. [문제 발생 히스토리](#문제-발생-히스토리)
3. [최종 아키텍처](#최종-아키텍처)
4. [구현 상세](#구현-상세)
5. [트러블슈팅 과정](#트러블슈팅-과정)
6. [테스트 가이드](#테스트-가이드)
7. [핵심 학습 사항](#핵심-학습-사항)
8. [참고 자료](#참고-자료)

---

## 개요

### 배경

YiSan 프로젝트의 멀티플레이어 환경에서 **플레이어 접속 순서를 보장하는 고유 인덱스(PlayerIndex)** 관리 시스템 구축 과정에서 발생한 일련의 문제들과 최종 해결 방안을 정리한 통합 보고서입니다.

### 요구사항

```cpp
AYiSanPlayerState::PlayerIndex
```

- **접속 순서 보장**: 0, 1, 2, 3, ... (중복 없이 순차적으로 할당)
- **맵 전환 시 유지**: Seamless Travel 시에도 PlayerIndex 보존
- **네트워크 동기화**: 서버에서 할당, 클라이언트로 자동 복제
- **UI 표시**: 자신의 이름은 녹색, 다른 플레이어는 흰색으로 표시

### 문제 요약

| 세션 | 문제 | 원인 | 해결책 |
|------|------|------|--------|
| **1차** (2025-11-01) | Guest 유저 이름이 녹색으로 표시 안 됨 | GetFirstPlayerController() 사용 | GetOwningPlayer() 변경 |
| **2차** (2025-11-02 AM) | PlayerIndex 2,3으로 시작 (0,1이 아님) | GameInstance 값이 PIE 세션 간 유지됨 | GameState로 이동 |
| **3차** (2025-11-02 PM) | Seamless Travel 시 Host PlayerIndex가 -1로 리셋 | PlayerState 재생성, PostLogin 미호출 | HandleSeamlessTravelPlayer 구현 |

---

## 문제 발생 히스토리

### 세션 1: UI 색상 표시 문제 (2025-11-01)

#### 증상
```
호스트: 자신의 이름 녹색 ✅
Guest: 자신의 이름 흰색으로 표시 ❌
```

#### 로그 분석
```
Name: www, InPlayerIndex: 1, LocalPlayerIndex: 0, IsLocal: FALSE (흰색)
                                              ↑ 호스트의 인덱스!
```

#### 원인

1. **GetFirstPlayerController() 사용**
   ```cpp
   // ❌ 잘못된 코드
   APlayerController* PC = GetWorld()->GetFirstPlayerController();
   // 호스트: 자기 자신 반환 ✅
   // 클라이언트: 첫 번째 PC = 호스트 반환 ❌
   ```

2. **PlayerState 리플리케이션 타이밍**
   ```
   [UStartWidget] WARNING: LocalPS is NULL! PC = BP_GamePlayerCotrol_C_0
   ```

3. **빈 PlayerList로 인한 UI 초기화**
   ```
   UpdatePlayerList(2명) → 표시 ✅
   UpdatePlayerList(0명) → ClearChildren() → 화면 비워짐 ❌
   ```

#### 해결책

1. **GetOwningPlayer() 사용**
   ```cpp
   // ✅ 올바른 코드
   APlayerController* PC = GetOwningPlayer();
   ```

2. **재시도 로직 추가**
   ```cpp
   if (!LocalPS)
   {
       FTimerHandle RetryHandle;
       GetWorld()->GetTimerManager().SetTimer(RetryHandle, [this, playerNames]()
       {
           UpdatePlayerList(playerNames);
       }, 0.1f, false);
       return;
   }
   ```

3. **빈 배열 필터링**
   ```cpp
   if (playerNames.Num() == 0)
   {
       PRINTLOG(TEXT("PlayerNames is empty - skipping update"));
       return;
   }
   ```

---

### 세션 2: GameInstance → GameState 리팩토링 (2025-11-02 AM)

#### 증상
```
로그:
  Player 0: 2:qqq
  Player 1: 3:www
  LocalPlayerIndex = 3

예상: PlayerIndex 0, 1
실제: PlayerIndex 2, 3
```

#### 원인

**GameInstance의 NextPlayerIndex가 PIE 세션 간에 유지됨**

```
첫 번째 PIE 세션:
  Host: PlayerIndex = 0 (NextPlayerIndex: 0 → 1)
  Guest: PlayerIndex = 1 (NextPlayerIndex: 1 → 2)

PIE 종료 (GameInstance는 살아있음, NextPlayerIndex = 2 유지)

두 번째 PIE 세션:
  Host: PlayerIndex = 2 ❌ (NextPlayerIndex: 2 → 3)
  Guest: PlayerIndex = 3 ❌ (NextPlayerIndex: 3 → 4)
```

#### 해결책

**GameInstance → GameState로 이동**

| | GameInstance | GameState |
|---|---|------|
| 복제 | ❌ 안 됨 | ✅ 됨 |
| Seamless Travel | ✅ 유지 | ✅ 유지 |
| PIE 세션 격리 | ❌ (유지될 수 있음) | ✅ (항상 새로 생성) |
| 서버/클라 존재 | 둘 다 | 둘 다 |
| 용도 | 세션, 설정 관리 | **게임 상태, 플레이어 관리** |

**구현:**

```cpp
// AYisanGameState.h
class AYisanGameState : public AGameStateBase
{
public:
    /** @brief 다음 PlayerIndex 값 (전역 static) */
    static int32 NextPlayerIndex;
};

// AYisanGameState.cpp
int32 AYisanGameState::NextPlayerIndex = 0;
```

---

### 세션 3: Seamless Travel PlayerState 소실 (2025-11-02 PM)

#### 증상
```
StartLevel: Host PlayerIndex = 0 ✅
MainMap: Host PlayerIndex = -1 ❌ (새 PlayerState 생성!)
```

#### 로그 분석
```
[11:44:01] StartLevel PostLogin: PlayerState=BP_YiSanPlayerState_C_0, PlayerIndex=0
[11:44:06] MainMap HandleSeamlessTravelPlayer: PlayerState=BP_YiSanPlayerState_C_1, PlayerIndex=-1
```

**핵심 발견**: PlayerState 객체 이름이 `_C_0` → `_C_1`로 변경됨

#### 원인

1. **Seamless Travel 시 PlayerState 재생성**
   - Unreal Engine 내부 동작으로 PlayerState가 새로 생성될 수 있음
   - 이전 PlayerIndex 값 손실

2. **Host PostLogin 미호출**
   - Seamless Travel 시 Host는 기존 접속 유지
   - PostLogin은 새로 접속하는 플레이어(Guest)만 호출
   - Host는 `HandleSeamlessTravelPlayer`만 호출됨

3. **BeginPlay vs PostLogin 호출 순서**
   ```
   [11:30:22.489] PostLogin - PlayerIndex 0 할당
   [11:30:22.492] BeginPlay - NextPlayerIndex = 0 리셋 ❌
   ```

4. **NextPlayerIndex 증가 문제**
   ```
   StartLevel: NextPlayerIndex = 1 (Host=0 할당 후)
   MainMap HandleSeamlessTravelPlayer: NextPlayerIndex++ → Host=1 할당 ❌
   ```

#### 해결책

**1. InitGame으로 초기화 이동**

```cpp
void AYiSanGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // InitGame은 PostLogin보다 먼저 호출됨
    AYisanGameState::NextPlayerIndex = 0;
    PRINTLOG(TEXT("[GameMode] InitGame - Reset NextPlayerIndex to 0 (Map: %s)"), *MapName);
}
```

**호출 순서**: InitGame → HandleSeamlessTravelPlayer → PostLogin → BeginPlay

**2. HandleSeamlessTravelPlayer 구현**

```cpp
void AYiSanGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

    if (APlayerController* PC = Cast<APlayerController>(C))
    {
        if (AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(PC->PlayerState))
        {
            // Seamless Travel로 이동한 플레이어(Host)에게 PlayerIndex 할당
            if (PS->PlayerIndex < 0)
            {
                int32 AssignedIndex = AYisanGameState::NextPlayerIndex++;
                PS->SetPlayerIndex(AssignedIndex);
                PRINTLOG(TEXT("[GameMode] HandleSeamlessTravelPlayer - Assigned PlayerIndex %d"), AssignedIndex);
            }
        }
    }
}
```

**3. PostLogin 중복 방지**

```cpp
void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(NewPlayer->PlayerState);
    if (!PS) return;

    // 새로 접속하는 플레이어(Guest)에게만 할당
    if (PS->PlayerIndex < 0)
    {
        int32 AssignedIndex = AYisanGameState::NextPlayerIndex++;
        PS->SetPlayerIndex(AssignedIndex);
        PRINTLOG(TEXT("[GameMode] Assigned PlayerIndex %d"), AssignedIndex);
    }
    else
    {
        PRINTLOG(TEXT("[GameMode] PlayerIndex already set to %d - skipping"), PS->PlayerIndex);
    }
}
```

**4. PlayerState 기본값 -1**

```cpp
AYiSanPlayerState::AYiSanPlayerState()
    : PlayerIndex(-1)  // 명시적 초기화
{
}
```

---

## 최종 아키텍처

### 시스템 구조도

```
┌─────────────────────────────────────────────────────────────┐
│                   AYisanGameState                           │
│  static int32 NextPlayerIndex (프로세스 전역)               │
│                                                             │
│  [Seamless Travel 시 값 유지]                               │
│  [각 레벨 InitGame에서 0으로 리셋]                          │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ 사용
                              ▼
        ┌─────────────────────────────────────────┐
        │                                         │
        ▼                                         ▼
┌──────────────────┐                   ┌──────────────────┐
│ ALobbyGameMode   │  Seamless Travel  │ AYiSanGameMode   │
│  (로비 전용)     │──────────────────▶│  (메인 게임)     │
│                  │                   │                  │
│ PostLogin():     │                   │ InitGame():      │
│   (건너뜀)       │                   │   Reset to 0     │
│                  │                   │                  │
│                  │                   │ HandleSeamless   │
│                  │                   │ TravelPlayer():  │
│                  │                   │   할당 (Host)    │
│                  │                   │                  │
│                  │                   │ PostLogin():     │
│                  │                   │   할당 (Guest)   │
└──────────────────┘                   └──────────────────┘
        │                                         │
        │ PlayerIndex 무시                        │ PlayerIndex 할당
        ▼                                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  AYiSanPlayerState                          │
│  UPROPERTY(ReplicatedUsing=OnRep_PlayerIndex)               │
│  int32 PlayerIndex = -1  (기본값)                           │
│                                                             │
│  [서버에서 할당 → 자동으로 클라이언트에 복제됨]              │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ 복제
                              ▼
        ┌─────────────────────────────────────────┐
        │                                         │
        ▼                                         ▼
┌──────────────────┐                   ┌──────────────────┐
│ UStartWidget     │                   │ UPlayerWidget    │
│                  │                   │                  │
│ GetOwningPlayer()│                   │ GetOwningPlayer()│
│ LocalPlayerIndex │                   │ LocalPlayerIndex │
└──────────────────┘                   └──────────────────┘
        │                                         │
        ▼                                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  UPlayerListItem                            │
│  SetPlayerStatus(InIndex, LocalIndex, Name):                │
│    if (InIndex == LocalIndex) → 녹색                        │
│    else → 흰색                                              │
└─────────────────────────────────────────────────────────────┘
```

### GameMode 생명주기

```
InitGame (제일 먼저)
  ↓ NextPlayerIndex = 0
  ↓
HandleSeamlessTravelPlayer (Seamless Travel 플레이어)
  ↓ Host에게 PlayerIndex 할당
  ↓
PostLogin (새 접속자)
  ↓ Guest에게 PlayerIndex 할당
  ↓
BeginPlay (제일 나중)
```

---

## 구현 상세

### 1. AYisanGameState

**Header (AYisanGameState.h)**
```cpp
class YISAN_API AYisanGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    /** @brief 다음 플레이어 인덱스 (전역 static) */
    static int32 NextPlayerIndex;
};
```

**Implementation (AYisanGameState.cpp)**
```cpp
// Static 변수 정의
int32 AYisanGameState::NextPlayerIndex = 0;
```

---

### 2. AYiSanGameMode

**Header (AYiSanGameMode.h)**
```cpp
class AYiSanGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void HandleSeamlessTravelPlayer(AController*& C) override;
};
```

**Implementation (AYiSanGameMode.cpp)**
```cpp
void AYiSanGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Reset PlayerIndex counter at the start of each level
    AYisanGameState::NextPlayerIndex = 0;
    PRINTLOG(TEXT("[GameMode] InitGame - Reset NextPlayerIndex to 0 (Map: %s)"), *MapName);
}

void AYiSanGameMode::BeginPlay()
{
    Super::BeginPlay();
    bUseSeamlessTravel = true;

    if (HasAuthority())
    {
        // QuestManager 초기화 등...
    }
}

void AYiSanGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

    if (APlayerController* PC = Cast<APlayerController>(C))
    {
        if (AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(PC->PlayerState))
        {
            PRINTLOG(TEXT("[GameMode] HandleSeamlessTravelPlayer - PC=%s, PlayerState=%s, PlayerIndex=%d (before)"),
                *GetNameSafe(PC), *GetNameSafe(PS), PS->PlayerIndex);

            // Seamless Travel로 이동한 플레이어(Host)에게 PlayerIndex 할당
            if (PS->PlayerIndex < 0)
            {
                int32 AssignedIndex = AYisanGameState::NextPlayerIndex++;
                PS->SetPlayerIndex(AssignedIndex);
                PRINTLOG(TEXT("[GameMode] HandleSeamlessTravelPlayer - Assigned PlayerIndex %d"), AssignedIndex);
            }
            else
            {
                PRINTLOG(TEXT("[GameMode] HandleSeamlessTravelPlayer - PlayerIndex already set, skipping"));
            }
        }
    }
}

void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    PRINTLOG(TEXT("[GameMode] PostLogin - PC=%s Pawn=%s PlayerState=%s"),
        *GetNameSafe(NewPlayer),
        *GetNameSafe(NewPlayer->GetPawn()),
        *GetNameSafe(NewPlayer->PlayerState));

    AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(NewPlayer->PlayerState);
    if (!PS)
    {
        PRINTLOG(TEXT("[GameMode] ERROR: PlayerState is null or cast failed!"));
        return;
    }

    // Set PlayerIndex using GameState's static counter (only if not already set)
    if (PS->PlayerIndex < 0)
    {
        int32 AssignedIndex = AYisanGameState::NextPlayerIndex++;
        PS->SetPlayerIndex(AssignedIndex);
        PRINTLOG(TEXT("[GameMode] Assigned PlayerIndex %d to %s (NextPlayerIndex is now %d)"),
            AssignedIndex, *GetNameSafe(NewPlayer), AYisanGameState::NextPlayerIndex);
    }
    else
    {
        PRINTLOG(TEXT("[GameMode] PlayerIndex already set to %d for %s - skipping assignment"),
            PS->PlayerIndex, *GetNameSafe(NewPlayer));
    }

    // ... DasanNPC 설정 등
}
```

---

### 3. ALobbyGameMode

**Implementation (ALobbyGameMode.cpp)**
```cpp
#include "ALobbyGameMode.h"
#include "APlayerControl.h"
#include "AYiSanPlayerState.h"
#include "AYisanGameState.h"
#include "GameLogging.h"

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    AYisanGameState::NextPlayerIndex = 0;
    PRINTLOG(TEXT("[LobbyGameMode] BeginPlay - Reset NextPlayerIndex to 0"));
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // Lobby is local-only, PlayerIndex will be assigned when joining host server
    PRINTLOG(TEXT("[LobbyGameMode] PostLogin - %s (PlayerIndex assignment skipped in lobby)"),
        *GetNameSafe(NewPlayer));

    // Pawn 생성...
}
```

---

### 4. AYiSanPlayerState

**Implementation (AYiSanPlayerState.cpp)**
```cpp
AYiSanPlayerState::AYiSanPlayerState()
    : PlayerIndex(-1)  // 명시적 초기화
{
}
```

---

### 5. UI Widgets

**UStartWidget.cpp / UPlayerWidget.cpp**
```cpp
void UStartWidget::UpdatePlayerList(const TArray<FString>& playerNames)
{
    if (!playerList) return;

    // 빈 배열 필터링
    if (playerNames.Num() == 0)
    {
        PRINTLOG(TEXT("[UStartWidget] PlayerNames is empty - skipping update"));
        return;
    }

    // GetOwningPlayer() 사용
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        PRINTLOG(TEXT("[UStartWidget] ERROR: PC is NULL!"));
        return;
    }

    // PlayerState 리플리케이션 대기
    AYiSanPlayerState* LocalPS = PC->GetPlayerState<AYiSanPlayerState>();
    if (!LocalPS)
    {
        PRINTLOG(TEXT("[UStartWidget] WARNING: LocalPS is NULL! Retrying in 0.1s..."));
        FTimerHandle RetryHandle;
        GetWorld()->GetTimerManager().SetTimer(RetryHandle, [this, playerNames]()
        {
            UpdatePlayerList(playerNames);
        }, 0.1f, false);
        return;
    }

    int32 LocalPlayerIndex = LocalPS->PlayerIndex;
    PRINTLOG(TEXT("[UStartWidget] LocalPlayerIndex = %d"), LocalPlayerIndex);

    playerList->ClearChildren();

    for (const FString& playerName : playerNames)
    {
        // ... UI 생성 로직
    }
}
```

---

## 트러블슈팅 과정

### 동작 흐름

#### Lobby (로컬 게임)
```
1. ALobbyGameMode::BeginPlay
   - NextPlayerIndex = 0 초기화 (무의미, 어차피 새 세션 시작)

2. ALobbyGameMode::PostLogin
   - PlayerIndex 할당 건너뜀 (로그만 출력)
```

#### StartLevel (호스트 서버 시작)
```
1. AYiSanGameMode::InitGame
   - NextPlayerIndex = 0 초기화

2. AYiSanGameMode::PostLogin (Host)
   - PlayerIndex < 0 → 0 할당 ✓
   - NextPlayerIndex: 0 → 1

3. AYiSanGameMode::PostLogin (Guest 접속 시)
   - PlayerIndex < 0 → 1 할당 ✓
   - NextPlayerIndex: 1 → 2
```

#### MainMap (Seamless Travel)
```
1. AYiSanGameMode::InitGame
   - NextPlayerIndex = 0 리셋

2. AYiSanGameMode::HandleSeamlessTravelPlayer (Host)
   - PlayerState 새로 생성됨 (PlayerIndex = -1)
   - PlayerIndex < 0 → 0 할당 ✓
   - NextPlayerIndex: 0 → 1

3. AYiSanGameMode::PostLogin (Guest)
   - PlayerIndex < 0 → 1 할당 ✓
   - NextPlayerIndex: 1 → 2
```

### 예상 로그

```
[LobbyGameMode] BeginPlay - Reset NextPlayerIndex to 0
[LobbyGameMode] PostLogin - BP_GamePlayerCotrol_C_0 (PlayerIndex assignment skipped in lobby)

[GameMode] InitGame - Reset NextPlayerIndex to 0 (Map: StartLevel)
[GameMode] PostLogin - PC=BP_GamePlayerCotrol_C_0 ...
[GameMode] Assigned PlayerIndex 0 to BP_GamePlayerCotrol_C_0 (NextPlayerIndex is now 1)

[GameMode] PostLogin - PC=BP_GamePlayerCotrol_C_2 ...
[GameMode] Assigned PlayerIndex 1 to BP_GamePlayerCotrol_C_2 (NextPlayerIndex is now 2)

[GameMode] InitGame - Reset NextPlayerIndex to 0 (Map: MainMap_WP)
[GameMode] HandleSeamlessTravelPlayer - PC=..., PlayerState=..., PlayerIndex=-1 (before)
[GameMode] HandleSeamlessTravelPlayer - Assigned PlayerIndex 0
[GameMode] PostLogin - PC=...
[GameMode] Assigned PlayerIndex 1

[UpdatePlayerList] Current players: 0:host, 1:guest ✓
```

---

## 테스트 가이드

### 시나리오 1: 로비 정상 접속

```
1. PIE 또는 Standalone으로 LobbyMap 시작

2. 로그 확인:
   [LobbyGameMode] BeginPlay - Reset NextPlayerIndex to 0

3. Host 접속 후 로그:
   [LobbyGameMode] PostLogin - (PlayerIndex assignment skipped in lobby)

4. Guest 접속 후 로그:
   [LobbyGameMode] PostLogin - (PlayerIndex assignment skipped in lobby)
```

### 시나리오 2: StartLevel 전환 및 할당

```
1. StartLevel 이동

2. 로그 확인:
   [GameMode] InitGame - Reset NextPlayerIndex to 0 (Map: StartLevel)
   [GameMode] Assigned PlayerIndex 0 to Host
   [GameMode] Assigned PlayerIndex 1 to Guest

3. UI 확인:
   Host 화면: qqq(녹색), www(흰색)
   Guest 화면: qqq(흰색), www(녹색)
```

### 시나리오 3: MainMap Seamless Travel

```
1. StartLevel → MainMap_WP Seamless Travel

2. 로그 확인:
   [GameMode] InitGame - Reset NextPlayerIndex to 0 (Map: MainMap_WP)
   [GameMode] HandleSeamlessTravelPlayer - Assigned PlayerIndex 0
   [GameMode] Assigned PlayerIndex 1

3. UI 확인:
   Host 화면: qqq(녹색), www(흰색)
   Guest 화면: qqq(흰색), www(녹색)

4. PlayerList 확인:
   0:qqq, 1:www ✓
```

### 시나리오 4: PIE 재시작 테스트

```
1. 첫 번째 PIE 세션 실행
   Host: PlayerIndex = 0
   Guest: PlayerIndex = 1

2. PIE 종료

3. 두 번째 PIE 세션 실행
   Host: PlayerIndex = 0 (다시 0부터 시작) ✓
   Guest: PlayerIndex = 1 ✓

검증: 이전 세션의 NextPlayerIndex가 유지되지 않음
```

---

## 핵심 학습 사항

### 1. GameMode 생명주기 이해

```
InitGame (제일 먼저)
  → 레벨별 초기화 작업
  → NextPlayerIndex = 0

HandleSeamlessTravelPlayer (Seamless Travel 플레이어)
  → Host가 이 경로로 들어옴
  → PlayerIndex 할당 필요

PostLogin (새 접속자)
  → Guest가 이 경로로 들어옴
  → PlayerIndex 할당 필요

BeginPlay (제일 나중)
  → 게임 로직 초기화
```

### 2. Seamless Travel의 함정

1. **PlayerState가 항상 유지되는 것은 아님**
   - Unreal Engine 내부 구현으로 재생성될 수 있음
   - 이전 데이터 손실 가능성

2. **Host는 PostLogin이 호출되지 않음**
   - 기존 접속 유지
   - `HandleSeamlessTravelPlayer`만 호출됨

3. **매 레벨마다 NextPlayerIndex 리셋 필요**
   - 순서 보장을 위해 0부터 다시 시작

### 3. Static 변수 관리

```cpp
// GameState에 선언
class AYisanGameState : public AGameStateBase
{
public:
    static int32 NextPlayerIndex;
};

// cpp 파일에 정의
int32 AYisanGameState::NextPlayerIndex = 0;
```

**장점:**
- 프로세스 전역으로 유지
- 여러 GameMode에서 공유
- PIE 세션마다 격리 (프로세스 재시작 시 초기화)

### 4. 네트워크 프로그래밍 원칙

```cpp
// ❌ 잘못된 코드
APlayerController* PC = GetWorld()->GetFirstPlayerController();
// 클라이언트에서 호스트의 PC를 가져옴

// ✅ 올바른 코드
APlayerController* PC = GetOwningPlayer();
// 위젯을 소유한 PC를 가져옴
```

### 5. 복제(Replication) 타이밍 고려

```cpp
// PlayerState가 아직 복제 안 됨 - 재시도
if (!LocalPS)
{
    FTimerHandle RetryHandle;
    GetWorld()->GetTimerManager().SetTimer(RetryHandle, [this, data]()
    {
        RetryUpdate(data);
    }, 0.1f, false);
    return;
}
```

---

## 수정 파일 목록

### 필수 수정

1. **AYisanGameState.h** (56줄)
   - `static int32 NextPlayerIndex` 추가

2. **AYisanGameState.cpp** (17줄)
   - `int32 AYisanGameState::NextPlayerIndex = 0;` 정의

3. **AYiSanGameMode.h** (19-22줄)
   - `InitGame()` 함수 선언
   - `HandleSeamlessTravelPlayer()` 함수 선언

4. **AYiSanGameMode.cpp** (13-89줄)
   - `InitGame()` 구현
   - `HandleSeamlessTravelPlayer()` 구현
   - `PostLogin()` 중복 할당 방지 로직

5. **AYiSanPlayerState.cpp** (7-9줄)
   - 생성자에서 `PlayerIndex(-1)` 초기화

6. **ALobbyGameMode.cpp** (8줄, 28-44줄)
   - `GameLogging.h` include
   - PostLogin에서 PlayerIndex 할당 제거

7. **UStartWidget.cpp**
   - `GetFirstPlayerController()` → `GetOwningPlayer()`
   - PlayerState NULL 체크 및 재시도 로직
   - 빈 배열 필터링

8. **UPlayerWidget.cpp**
   - `GetFirstPlayerController()` → `GetOwningPlayer()`
   - PlayerState NULL 체크 및 재시도 로직
   - 빈 배열 필터링

### 제거 가능 (선택사항)

**UYiSanGameInstance.h/cpp**
```cpp
// 더 이상 사용하지 않음 (삭제 가능)
int32 GetNextPlayerIndex();
void ResetPlayerIndex();
int32 NextPlayerIndex = 0;
```

---

## 참고 자료

### Unreal Engine 공식 문서
- [GameState](https://docs.unrealengine.com/5.3/en-US/game-state-in-unreal-engine/)
- [GameMode](https://docs.unrealengine.com/5.3/en-US/game-mode-in-unreal-engine/)
- [PlayerState Replication](https://docs.unrealengine.com/5.3/en-US/player-state-in-unreal-engine/)
- [Seamless Travel](https://docs.unrealengine.com/5.3/en-US/travelling-in-multiplayer-in-unreal-engine/)
- [Replication](https://docs.unrealengine.com/5.3/en-US/replication-in-unreal-engine/)

### 관련 문서
- `Documents/DevLog/PlayerIndex_Fix_Report.md` - 세션 1 (UI 색상 문제)
- `Documents/DevLog/PlayerIndex_GameState_Refactoring_2025-11-02.md` - 세션 2 (GameState 리팩토링)
- `Documents/DevLog/PlayerIndex_Issue_2025-11-02.md` - 세션 3 (Seamless Travel 문제)

---

## 결론

### 최종 해결 사항

✅ **PlayerIndex 할당**: 접속 순서대로 0, 1, 2, ... 보장
✅ **Seamless Travel**: 레벨 전환 시 순서 유지
✅ **UI 색상 표시**: 자신은 녹색, 다른 플레이어는 흰색
✅ **네트워크 동기화**: 서버 할당 → 클라이언트 복제
✅ **PIE 세션 격리**: 재시작 시 0부터 다시 시작

### 핵심 패턴

1. **Static 변수를 GameState에**: 프로세스 전역 + PIE 격리
2. **InitGame에서 초기화**: PostLogin보다 먼저 호출
3. **HandleSeamlessTravelPlayer 구현**: Host PlayerIndex 할당
4. **PostLogin 중복 방지**: `PlayerIndex < 0` 체크
5. **GetOwningPlayer() 사용**: 클라이언트별 올바른 PC 획득

### 작성자 노트

이 문제는 멀티플레이어 네트워크 프로그래밍의 핵심 개념들이 복합적으로 얽힌 사례였습니다:

1. **GameMode 생명주기**: InitGame → HandleSeamlessTravelPlayer → PostLogin → BeginPlay
2. **Seamless Travel 특성**: PlayerState 재생성, Host PostLogin 미호출
3. **Static 변수 관리**: 파일 스코프 → GameState 전역
4. **네트워크 동기화**: 서버 권한, 클라이언트 복제, 타이밍 이슈

3개 세션에 걸쳐 단계적으로 문제를 해결하며 Unreal Engine의 멀티플레이어 아키텍처를 깊이 이해할 수 있었습니다.

---

**문서 버전:** 1.0 Final
**작성자:** Claude Code
**최종 수정:** 2025-11-02 12:00 KST
