# PlayerIndex 할당 및 UI 색상 표시 문제 해결 보고서

**작성일:** 2025-11-01
**프로젝트:** YiSan
**상태:** ✅ 해결 완료

---

## 📋 목차
1. [문제 상황](#문제-상황)
2. [원인 분석](#원인-분석)
3. [해결 과정](#해결-과정)
4. [최종 구현](#최종-구현)
5. [테스트 결과](#테스트-결과)
6. [수정 파일 목록](#수정-파일-목록)

---

## 🚨 문제 상황

### 증상
- **호스트**: 자신의 이름만 녹색으로 표시 (정상 ✅)
- **클라이언트(Guest)**: 자신의 이름이 녹색으로 표시되지 않음 (오류 ❌)

### 요구사항
```cpp
AYiSanPlayerState::PlayerIndex
```
- 접속 순서를 보장하는 고유한 인덱스 (0, 1, 2, ...)
- 이 값을 기반으로 `UPlayerListItem`의 `PlayerNameText` 색상 결정
  - **나라면**: 녹색 (`FLinearColor::Green`)
  - **다른 플레이어**: 흰색 (`FLinearColor::White`)

---

## 🔍 원인 분석

### 1차 원인: ALobbyGameMode에 PlayerIndex 할당 로직 누락

**AYiSanGameMode (메인맵)**
```cpp
// ✅ PlayerIndex 할당 코드 존재
static int32 NextPlayerIndex = 0;

void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
    if (PS->PlayerIndex == -1)
        PS->PlayerIndex = NextPlayerIndex++;
}
```

**ALobbyGameMode (로비)**
```cpp
// ❌ PlayerIndex 할당 코드 없음!
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    // PlayerIndex 할당 로직 누락
}
```

**결과:** 모든 플레이어의 PlayerIndex가 초기값 `-1`로 유지됨

---

### 2차 원인: static 변수의 파일 스코프 문제

각 `.cpp` 파일의 `static int32 NextPlayerIndex`는 **internal linkage**로 인해 완전히 독립적인 변수입니다.

```
ALobbyGameMode.cpp의 NextPlayerIndex: 메모리 주소 0x12345000
AYiSanGameMode.cpp의 NextPlayerIndex: 메모리 주소 0x67890000
```

**시나리오:**
```
로비 맵 (ALobbyGameMode):
  플레이어1 → PlayerIndex = 0
  플레이어2 → PlayerIndex = 1

메인 맵 전환 (Seamless Travel):
  AYiSanGameMode::NextPlayerIndex = 0 (별개 변수)

  플레이어3 난입:
    PlayerIndex = NextPlayerIndex++ → 0  ❌❌

결과: 플레이어1(0), 플레이어2(1), 플레이어3(0) → 중복!
```

---

### 3차 원인: GetFirstPlayerController() 사용

**잘못된 코드:**
```cpp
APlayerController* PC = GetWorld()->GetFirstPlayerController();
```

- **호스트**: 자기 자신 반환 (PlayerIndex 0) ✅
- **클라이언트**: **첫 번째 PC = 호스트** 반환 (PlayerIndex 0) ❌

**로그 증거:**
```
Name: www, InPlayerIndex: 1, LocalPlayerIndex: 0, IsLocal: FALSE (흰색)
                                              ↑ 호스트의 인덱스!
```

---

### 4차 원인: PlayerState 리플리케이션 타이밍 문제

클라이언트에서 UI 업데이트 시 PlayerState가 아직 리플리케이션되지 않음:

```
[UStartWidget] WARNING: LocalPS is NULL! PC = BP_GamePlayerCotrol_C_0
```

---

### 5차 원인: 빈 PlayerList로 인한 UI 초기화

```
UpdatePlayerList(2명) → 화면에 표시 ✅
UpdatePlayerList(0명) → ClearChildren() 호출 → 화면 비워짐 ❌
```

---

## 🛠️ 해결 과정

### 1단계: GameInstance에 PlayerIndex 카운터 통합

**문제:** static 변수의 파일 스코프로 인한 독립성
**해결:** GameInstance를 사용하여 단일 진실 공급원(Single Source of Truth) 구현

#### 수정: UYiSanGameInstance.h
```cpp
UCLASS()
class YISAN_API UYiSanGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    /** 플레이어 접속 순서를 보장하는 인덱스를 할당받습니다 (0, 1, 2, ...) */
    UFUNCTION(BlueprintCallable, Category = "Player")
    int32 GetNextPlayerIndex() { return NextPlayerIndex++; }

    /** 게임 시작 시 PlayerIndex 카운터를 초기화합니다 */
    UFUNCTION(BlueprintCallable, Category = "Player")
    void ResetPlayerIndex() { NextPlayerIndex = 0; }

private:
    /** 다음에 할당할 PlayerIndex (맵 전환 시에도 유지됨) */
    int32 NextPlayerIndex = 0;
};
```

**장점:**
- ✅ 맵 전환 시에도 자동 유지
- ✅ 모든 GameMode에서 동일한 카운터 공유
- ✅ static 변수 제거로 복잡도 감소

---

### 2단계: ALobbyGameMode에 PlayerIndex 할당 로직 추가

#### 수정: ALobbyGameMode.cpp
```cpp
void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // GameInstance의 PlayerIndex 카운터 초기화
    if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
    {
        GI->ResetPlayerIndex();
        PRINTLOG(TEXT("[LobbyGameMode] PlayerIndex counter reset"));
    }
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(NewPlayer->PlayerState);
    if (!PS) return;

    // PlayerIndex 할당 (접속 순서 보장)
    if (PS->PlayerIndex == -1)
    {
        if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
        {
            PS->PlayerIndex = GI->GetNextPlayerIndex();
            PRINTLOG(TEXT("[LobbyGameMode] Assigned PlayerIndex %d to %s"),
                     PS->PlayerIndex, *PS->GetPlayerName());
        }
    }
}
```

---

### 3단계: AYiSanGameMode 간소화

#### 수정: AYiSanGameMode.cpp
```cpp
void AYiSanGameMode::BeginPlay()
{
    Super::BeginPlay();
    bUseSeamlessTravel = true;

    // GameInstance의 PlayerIndex는 로비에서 이미 할당되었으므로
    // Seamless Travel로 PlayerState가 유지되어 접속 순서가 보존됨
    // (초기화 불필요)
}

void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (AYiSanPlayerState* PS = NewPlayer->GetPlayerState<AYiSanPlayerState>())
    {
        if (PS->PlayerIndex == -1)  // 메인맵에서 난입한 경우만
        {
            if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
            {
                PS->PlayerIndex = GI->GetNextPlayerIndex();
                PRINTLOG(TEXT("[YiSanGameMode] Assigned PlayerIndex %d to %s (난입)"),
                         PS->PlayerIndex, *PS->GetPlayerName());
            }
        }
    }
}
```

---

### 4단계: GetOwningPlayer() 사용으로 변경

#### 수정: UStartWidget.cpp, UPlayerWidget.cpp
```cpp
// ❌ 잘못된 코드
APlayerController* PC = GetWorld()->GetFirstPlayerController();

// ✅ 올바른 코드
APlayerController* PC = GetOwningPlayer();
```

**효과:**
- 호스트: 자기 자신 반환 ✅
- 클라이언트: 자기 자신 반환 ✅

---

### 5단계: PlayerState 리플리케이션 대기 로직 추가

#### 수정: UStartWidget.cpp, UPlayerWidget.cpp
```cpp
if (AYiSanPlayerState* LocalPS = PC->GetPlayerState<AYiSanPlayerState>())
{
    LocalPlayerIndex = LocalPS->PlayerIndex;
}
else
{
    // PlayerState가 아직 리플리케이션 안됨 - 0.1초 후 재시도
    PRINTLOG(TEXT("[UStartWidget] WARNING: LocalPS is NULL! Retrying in 0.1s..."));
    FTimerHandle RetryHandle;
    GetWorld()->GetTimerManager().SetTimer(RetryHandle, [this, playerNames]()
    {
        UpdatePlayerList(playerNames);
    }, 0.1f, false);
    return; // 현재 업데이트 중단
}
```

---

### 6단계: 빈 배열 필터링

#### 수정: UStartWidget.cpp, UPlayerWidget.cpp
```cpp
void UpdatePlayerList(const TArray<FString>& playerNames)
{
    if (!playerList) return;

    // playerNames가 비어있으면 업데이트하지 않음 (기존 리스트 유지)
    if (playerNames.Num() == 0)
    {
        PRINTLOG(TEXT("[UStartWidget] PlayerNames is empty - skipping update"));
        return;
    }

    playerList->ClearChildren();
    // ...
}
```

---

## 📊 최종 구현

### 아키텍처 다이어그램

```
┌─────────────────────────────────────────────────────────────┐
│                    UYiSanGameInstance                       │
│  - NextPlayerIndex: int32 (맵 전환 시에도 유지)              │
│  + GetNextPlayerIndex(): int32                              │
│  + ResetPlayerIndex(): void                                 │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ 사용
                              ▼
        ┌─────────────────────────────────────────┐
        │                                         │
        ▼                                         ▼
┌──────────────────┐                   ┌──────────────────┐
│ ALobbyGameMode   │                   │ AYiSanGameMode   │
│                  │  Seamless Travel  │                  │
│ BeginPlay():     │──────────────────▶│ BeginPlay():     │
│   ResetIndex()   │                   │   (초기화 없음)  │
│                  │                   │                  │
│ PostLogin():     │                   │ PostLogin():     │
│   할당(0,1,2...) │                   │   난입시만 할당  │
└──────────────────┘                   └──────────────────┘
        │                                         │
        │ PlayerIndex 할당                        │
        ▼                                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  AYiSanPlayerState                          │
│  - PlayerIndex: int32 (Replicated)                          │
│  - Nickname: FString (Replicated)                           │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ 참조
                              ▼
        ┌─────────────────────────────────────────┐
        │                                         │
        ▼                                         ▼
┌──────────────────┐                   ┌──────────────────┐
│ UStartWidget     │                   │ UPlayerWidget    │
│                  │                   │                  │
│ UpdatePlayerList │                   │ UpdatePlayerList │
│   - GetOwning    │                   │   - GetOwning    │
│     Player()     │                   │     Player()     │
│   - 재시도 로직  │                   │   - 재시도 로직  │
│   - 빈배열 필터  │                   │   - 빈배열 필터  │
└──────────────────┘                   └──────────────────┘
        │                                         │
        │ SetPlayerStatus                         │
        ▼                                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  UPlayerListItem                            │
│  SetPlayerStatus(InIndex, LocalIndex, Name):                │
│    if (InIndex == LocalIndex) → 녹색                        │
│    else → 흰색                                              │
└─────────────────────────────────────────────────────────────┘
```

### 동작 흐름

```
┌─────────────────────────────────────────────────────────────┐
│                      로비 맵 시작                            │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
              ┌───────────────────────────┐
              │ GI->ResetPlayerIndex()    │
              │ NextPlayerIndex = 0       │
              └───────────────────────────┘
                              │
                              ▼
              ┌───────────────────────────┐
              │ 플레이어1 접속            │
              │ PlayerIndex = 0           │
              │ NextPlayerIndex = 1       │
              └───────────────────────────┘
                              │
                              ▼
              ┌───────────────────────────┐
              │ 플레이어2 접속            │
              │ PlayerIndex = 1           │
              │ NextPlayerIndex = 2       │
              └───────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│           메인 맵으로 Seamless Travel                        │
│  - GameInstance 유지 (NextPlayerIndex = 2)                  │
│  - PlayerState 유지 (플레이어1=0, 플레이어2=1)              │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
              ┌───────────────────────────┐
              │ 플레이어3 난입            │
              │ PlayerIndex = -1 체크     │
              │ → GI->GetNextPlayerIndex()│
              │ PlayerIndex = 2           │
              │ NextPlayerIndex = 3       │
              └───────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│             UI 업데이트 (각 클라이언트)                     │
│                                                             │
│  호스트 화면:                                               │
│    - GetOwningPlayer() → 호스트 PC                          │
│    - LocalPlayerIndex = 0                                   │
│    - 플레이어1(0) == 0 → 녹색 ✅                            │
│    - 플레이어2(1) != 0 → 흰색                               │
│    - 플레이어3(2) != 0 → 흰색                               │
│                                                             │
│  클라이언트1 화면:                                          │
│    - GetOwningPlayer() → 클라이언트1 PC                     │
│    - LocalPlayerIndex = 1                                   │
│    - 플레이어1(0) != 1 → 흰색                               │
│    - 플레이어2(1) == 1 → 녹색 ✅                            │
│    - 플레이어3(2) != 1 → 흰색                               │
│                                                             │
│  클라이언트2 화면:                                          │
│    - GetOwningPlayer() → 클라이언트2 PC                     │
│    - LocalPlayerIndex = 2                                   │
│    - 플레이어1(0) != 2 → 흰색                               │
│    - 플레이어2(1) != 2 → 흰색                               │
│    - 플레이어3(2) == 2 → 녹색 ✅                            │
└─────────────────────────────────────────────────────────────┘
```

---

## ✅ 테스트 결과

### 로그 분석

**성공적인 PlayerIndex 할당:**
```
[LobbyGameMode] Assigned PlayerIndex 0 to qqq
[LobbyGameMode] Assigned PlayerIndex 1 to www
```

**LocalPlayerIndex 올바르게 가져옴:**
```
[UStartWidget] LocalPlayerController: BP_GamePlayerCotrol_C_0,
               LocalPlayerState: BP_YiSanPlayerState_C_1,
               LocalPlayerIndex = 1, Nickname = www
```

**색상 정상 적용:**
```
[PlayerListItem] SetPlayerStatus - Name: qqq, InPlayerIndex: 0, LocalPlayerIndex: 1, IsLocal: FALSE (흰색)
[PlayerListItem] SetPlayerStatus - Name: www, InPlayerIndex: 1, LocalPlayerIndex: 1, IsLocal: TRUE (녹색)
```

### 시나리오별 테스트

| 시나리오 | 호스트 | 클라이언트1 | 클라이언트2 | 결과 |
|---------|-------|------------|------------|------|
| 로비에서 정상 접속 | 자신 녹색 ✅ | 자신 녹색 ✅ | - | ✅ 통과 |
| 메인맵으로 전환 | PlayerIndex 유지 ✅ | PlayerIndex 유지 ✅ | - | ✅ 통과 |
| 메인맵에서 난입 | - | - | 자신 녹색 ✅ | ✅ 통과 |
| PlayerState 지연 | 0.1초 후 재시도 ✅ | 0.1초 후 재시도 ✅ | - | ✅ 통과 |
| 빈 배열 수신 | 리스트 유지 ✅ | 리스트 유지 ✅ | - | ✅ 통과 |

---

## 📁 수정 파일 목록

### 신규 추가
- `E:\UE\YiSan\Documents\DevLog\PlayerIndex_Fix_Report.md` (본 문서)

### 수정된 파일

#### GameInstance
- `Source/YiSan/Environment/Public/UYiSanGameInstance.h`
  - `GetNextPlayerIndex()` 함수 추가
  - `ResetPlayerIndex()` 함수 추가
  - `NextPlayerIndex` 멤버 변수 추가

#### GameMode
- `Source/YiSan/Environment/Private/ALobbyGameMode.cpp`
  - BeginPlay에서 PlayerIndex 카운터 초기화
  - PostLogin에서 PlayerIndex 할당 로직 추가
  - static 변수 제거

- `Source/YiSan/Environment/Private/AYiSanGameMode.cpp`
  - BeginPlay 간소화 (복잡한 초기화 로직 제거)
  - PostLogin 간소화 (난입자만 처리)
  - static 변수 제거
  - `UYiSanGameInstance.h` include 추가

#### UI Widgets
- `Source/YiSan/UI/Private/UStartWidget.cpp`
  - `GetWorld()->GetFirstPlayerController()` → `GetOwningPlayer()` 변경
  - PlayerState NULL 체크 및 재시도 로직 추가
  - 빈 배열 필터링 추가
  - `TimerManager.h` include 추가
  - 상세 로그 추가

- `Source/YiSan/UI/Private/UPlayerWidget.cpp`
  - `GetWorld()->GetFirstPlayerController()` → `GetOwningPlayer()` 변경
  - PlayerState NULL 체크 및 재시도 로직 추가
  - 빈 배열 필터링 추가
  - `TimerManager.h` include 추가
  - 상세 로그 추가

- `Source/YiSan/UI/Private/UPlayerListItem.cpp`
  - 디버깅 로그 추가
  - `GameLogging.h` include 추가

---

## 🎯 핵심 학습 사항

### 1. GameInstance의 활용
- **맵 전환 시에도 유지**되는 특성을 활용
- 전역 상태 관리에 적합
- static 변수의 파일 스코프 문제 회피

### 2. 네트워크 프로그래밍 주의사항
- `GetFirstPlayerController()`는 **호스트 우선**으로 반환
- Widget에서는 항상 `GetOwningPlayer()` 사용
- PlayerState 리플리케이션 타이밍 고려 필수

### 3. 멀티플레이어 UI 업데이트
- 각 클라이언트마다 **독립적인 시점** 고려
- 빈 데이터로 인한 UI 깜빡임 방지
- 재시도 로직으로 리플리케이션 지연 대응

### 4. Seamless Travel 특성
- PlayerState는 자동으로 유지됨
- GameMode는 맵마다 새로 생성됨
- GameInstance만 완전히 유지됨

---

## 🚀 향후 개선 가능 사항

### 1. 재시도 횟수 제한
현재 0.1초 후 1회만 재시도합니다. 필요시 최대 재시도 횟수 제한:

```cpp
// Header
int32 RetryCount = 0;
static constexpr int32 MaxRetries = 5;

// Implementation
if (RetryCount < MaxRetries)
{
    RetryCount++;
    // 재시도 로직
}
```

### 2. PlayerIndex 재사용
플레이어가 나갔을 때 인덱스를 재사용하는 로직:

```cpp
class UYiSanGameInstance
{
    TArray<int32> AvailableIndices; // 재사용 가능한 인덱스

    int32 GetNextPlayerIndex()
    {
        if (AvailableIndices.Num() > 0)
            return AvailableIndices.Pop();
        return NextPlayerIndex++;
    }

    void ReleasePlayerIndex(int32 Index)
    {
        AvailableIndices.Add(Index);
    }
};
```

### 3. 블루프린트 접근성
현재 C++로만 구현되어 있으나, 필요시 블루프린트 이벤트 추가:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerIndexAssigned,
    int32, PlayerIndex, FString, Nickname);

UPROPERTY(BlueprintAssignable)
FOnPlayerIndexAssigned OnPlayerIndexAssigned;
```

---

## 📚 참고 자료

### Unreal Engine 공식 문서
- [Game Instance](https://docs.unrealengine.com/5.3/en-US/game-instance-in-unreal-engine/)
- [Seamless Travel](https://docs.unrealengine.com/5.3/en-US/travelling-in-multiplayer-in-unreal-engine/)
- [PlayerState Replication](https://docs.unrealengine.com/5.3/en-US/player-state-in-unreal-engine/)

### 관련 코드 패턴
- Singleton Pattern (GameInstance)
- Observer Pattern (Delegate Broadcasting)
- Retry Pattern (PlayerState 대기)

---

## ✍️ 작성자 노트

이 문제는 멀티플레이어 네트워크 프로그래밍의 여러 핵심 개념이 얽혀있는 복합적인 이슈였습니다:

1. **상태 관리**: GameInstance vs GameMode vs static 변수
2. **네트워크 동기화**: Replication 타이밍
3. **클라이언트 관점**: GetFirstPlayerController vs GetOwningPlayer
4. **UI 업데이트**: 빈 데이터 처리

각 문제를 단계적으로 해결하며 Unreal Engine의 멀티플레이어 아키텍처에 대한 이해를 깊게 할 수 있었습니다.

특히 **GameInstance의 활용**은 앞으로 세션 관리, 플레이어 통계, 지속적인 데이터 저장 등 다양한 곳에 응용할 수 있는 중요한 패턴입니다.

---

**문서 버전:** 1.0
**최종 수정:** 2025-11-01 23:50 KST
