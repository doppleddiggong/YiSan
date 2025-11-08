# PlayerIndex 재초기화 문제 트러블슈팅 (2025-11-02)

**작성일:** 2025-11-02
**상태:** 🔍 분석 완료, 수정 대기 중

---

## 📋 목차
1. [문제 상황](#문제-상황)
2. [현재 시스템 구조](#현재-시스템-구조)
3. [원인 분석](#원인-분석)
4. [해결 방안](#해결-방안)
5. [다음 세션 작업 계획](#다음-세션-작업-계획)

---

## 🚨 문 상황

### 증상

#### 1. Guest 유저 정보 미표시 (주요 버그)
- **환경**: Standalone + Seamless Travel
- **시나리오**:
  ```
  1. Host(qqq)가 LobbyMap에서 방 개설
  2. Guest(www)가 조인
  3. StartLevel로 이동
  4. LoadingTransitions를 거쳐 MainLevel_WP로 이동
  ```
- **결과**:
  - Host(qqq): 모든 유저 정보 정상 노출 ✅
  - Guest(www): 자기 정보는 안 보이고, Host(qqq) 정보만 노출 ❌

#### 2. PlayerIndex 초기화 문제
- **증상**: MainLevel_WP 이동 후 PlayerIndex가 다시 초기화됨
- **현상**:
  ```
  LobbyMap → StartLevel:
    Host(qqq): PlayerIndex = 0
    Guest(www): PlayerIndex = 1

  StartLevel → MainLevel_WP:
    Host(qqq): PlayerIndex = 0 (?) 재할당
    Guest(www): PlayerIndex = ? (재할당됨, 잘못된 값)
  ```
- **단, PlayerName은 유지됨** (중요한 힌트!)

#### 3. LoadingTransitions 미표시 (부수 문제)
- **환경**: Standalone 모드
- **증상**: LoadingTransitions 위젯이 아예 노출되지 않음
- **참고**: Play In Editor에서는 정상 작동 가능성 있음

---

## 🏗️ 현재 시스템 구조

### 아키텍처 개요

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
│ ABaseGameMode    │                   │ ALobbyGameMode   │
│                  │                   │  (상속)          │
│ PostLogin():     │                   │                  │
│   if (PI == -1)  │                   │ BeginPlay():     │
│     할당         │                   │   ResetIndex()   │
└──────────────────┘                   └──────────────────┘
         │
         ▼
┌──────────────────┐
│ AYiSanGameMode   │
│  (상속)          │
│                  │
│ BeginPlay():     │
│   초기화 없음    │
└──────────────────┘
```

### 클래스별 역할

#### 1. UYiSanGameInstance
- **위치**: `Source/YiSan/Environment/Public/UYiSanGameInstance.h`
- **역할**: PlayerIndex 카운터 관리 (맵 전환 시에도 유지)
```cpp
class UYiSanGameInstance : public UGameInstance
{
    int32 NextPlayerIndex = 0;

    int32 GetNextPlayerIndex() { return NextPlayerIndex++; }
    void ResetPlayerIndex() { NextPlayerIndex = 0; }
};
```

#### 2. ABaseGameMode (신규 추가됨)
- **위치**: `Source/YiSan/Environment/Private/ABaseGameMode.cpp`
- **역할**: 모든 GameMode의 공통 기능
```cpp
void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AYiSanPlayerState* PS = NewPlayer->GetPlayerState<AYiSanPlayerState>();
    if (PS && PS->PlayerIndex == -1)
    {
        if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
        {
            PS->PlayerIndex = GI->GetNextPlayerIndex();
            PRINTLOG(TEXT("[BaseGameMode] Assigned NEW PlayerIndex %d to %s"),
                PS->PlayerIndex, *PS->GetPlayerName());
        }
    }
    else
    {
        PRINTLOG(TEXT("[BaseGameMode] PlayerIndex ALREADY SET: %d (Seamless Travel 성공)"),
            PS->PlayerIndex);
    }
}
```

#### 3. ALobbyGameMode
- **위치**: `Source/YiSan/Environment/Private/ALobbyGameMode.cpp`
- **역할**: 로비 전용 로직 + PlayerIndex 초기화
```cpp
void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // LobbyMap에서만 PlayerIndex 카운터 초기화
    if (CurrentMapName.Contains(TEXT("LobbyMap")) || ...)
    {
        if (GS->PlayerArray.Num() == 0)
        {
            if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
            {
                GI->ResetPlayerIndex();
            }
        }
    }
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    // BaseGameMode가 PlayerIndex 할당 처리
    Super::PostLogin(NewPlayer);

    // 로비 전용 로직 (플레이어 수 카운트 등)
}
```

#### 4. AYiSanGameMode
- **위치**: `Source/YiSan/Environment/Private/AYiSanGameMode.cpp`
- **역할**: 메인 게임 로직 (PlayerIndex는 자동 유지)
```cpp
void AYiSanGameMode::BeginPlay()
{
    Super::BeginPlay();
    bUseSeamlessTravel = true;

    // PlayerIndex 초기화 하지 않음
    // Seamless Travel로 PlayerState가 유지되어 접속 순서 보존됨
}

void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
    // BaseGameMode가 PlayerIndex 할당 처리
    Super::PostLogin(NewPlayer);

    // 메인 게임 전용 로직 (DasanNPC 설정 등)
}
```

#### 5. APlayerControl
- **위치**: `Source/YiSan/Character/Private/APlayerControl.cpp`
- **문제 지점**: `ServerRPC_SetPlayerNickname_Implementation`
```cpp
void APlayerControl::ServerRPC_SetPlayerNickname_Implementation(const FString& Nickname)
{
    if (AYiSanPlayerState* YSPlayerState = GetPlayerState<AYiSanPlayerState>())
    {
        // ⚠️ 문제: GameState의 PlayerArray.Num()으로 PlayerIndex를 재계산
        if (AGameStateBase* GS = GetWorld()->GetGameState())
        {
            int32 NewIndex = GS->PlayerArray.Num() - 1;  // ❌ 잘못된 계산!
            YSPlayerState->SetPlayerInfo(Nickname, NewIndex);

            // PlayerListManager 업데이트 요청
            if (AYiSanPlayerListManager* PlayerListManager = ...)
            {
                PlayerListManager->UpdatePlayerListAndBroadcast();
            }
        }
    }
}
```

#### 6. AYiSanPlayerState
- **위치**: `Source/YiSan/Environment/Private/AYiSanPlayerState.cpp`
```cpp
void AYiSanPlayerState::SetPlayerInfo(const FString& InName, int32 InIndex)
{
    if (HasAuthority())
    {
        Nickname = InName;
        // PlayerIndex = InIndex;  // ⚠️ 주석 처리되어 있음!
    }
}
```

---

## 🔍 원인 분석

### 문제 1: ServerRPC_SetPlayerNickname의 잘못된 PlayerIndex 계산

#### 현재 로직
```cpp
// APlayerControl.cpp:206-222
int32 NewIndex = GS->PlayerArray.Num() - 1;
YSPlayerState->SetPlayerInfo(Nickname, NewIndex);
```

#### 왜 문제인가?

1. **LobbyMap/StartLevel에서:**
   ```
   Host(qqq) 접속 시:
     - PostLogin 호출 → PlayerIndex = 0 할당 (BaseGameMode)
     - BeginPlay → ServerRPC_SetPlayerNickname 호출
     - PlayerArray.Num() = 1 → NewIndex = 0 ✅

   Guest(www) 접속 시:
     - PostLogin 호출 → PlayerIndex = 1 할당 (BaseGameMode)
     - BeginPlay → ServerRPC_SetPlayerNickname 호출
     - PlayerArray.Num() = 2 → NewIndex = 1 ✅
   ```

2. **MainLevel_WP 이동 후:**
   ```
   Seamless Travel 완료:
     - PlayerState는 유지됨 (PlayerIndex = 0, 1)

   OnPossess 호출 → ServerRPC_SetPlayerNickname 재호출:
     Host:
       - PlayerArray.Num() = 2 → NewIndex = 1 ❌ (원래는 0)

     Guest:
       - PlayerArray.Num() = ? → NewIndex = ? ❌
       - PlayerArray 순서가 네트워크 순서에 따라 다를 수 있음
   ```

3. **실제 실행 순서:**
   ```
   1. BaseGameMode::PostLogin → PlayerIndex 올바르게 할당 ✅
   2. APlayerControl::BeginPlay → ServerRPC_SetPlayerNickname 호출
   3. ServerRPC에서 PlayerArray.Num()-1로 재계산 ❌
   4. SetPlayerInfo 호출 → PlayerIndex 덮어쓰기 시도
   ```

#### 추가 발견: SetPlayerInfo가 PlayerIndex를 설정하지 않음

```cpp
// AYiSanPlayerState.cpp:12-19
void AYiSanPlayerState::SetPlayerInfo(const FString& InName, int32 InIndex)
{
    if (HasAuthority())
    {
        Nickname = InName;
        // PlayerIndex = InIndex;  // 주석 처리되어 있음!
    }
}
```

**의미:**
- 현재는 `SetPlayerInfo`가 PlayerIndex를 실제로 덮어쓰지 않음
- 하지만 잘못된 로직이 존재하고 있어, 나중에 주석 해제 시 버그 발생 가능성

### 문제 2: ServerRPC_SetPlayerNickname가 왜 재호출되는가?

#### OnPossess에서 호출
```cpp
// APlayerControl.cpp:164-187
void APlayerControl::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (IsLocalController())
    {
        if (UYiSanGameInstance* GI = GetGameInstance<UYiSanGameInstance>())
        {
            if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GI->GetSubsystem<...>())
            {
                const FString Nickname = NetworkSubsystem->GetPlayerNickname();
                if (!Nickname.IsEmpty())
                {
                    ServerRPC_SetPlayerNickname(Nickname);  // ⚠️ 재호출
                }
            }
        }
    }
}
```

#### BeginPlay에서 호출
```cpp
// APlayerControl.cpp:72-102
void APlayerControl::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        // ...

        // 클라이언트에서 닉네임을 서버로 전송
        if (UYiSanGameInstance* GI = GetGameInstance<UYiSanGameInstance>())
        {
            if (UNetworkGameInstanceSubsystem* NetworkSubsystem = ...)
            {
                FString Nickname = NetworkSubsystem->GetPlayerNickname();
                if (!Nickname.IsEmpty())
                {
                    ServerRPC_SetPlayerNickname(Nickname);  // ⚠️ 호출
                }
            }
        }
    }
}
```

#### 호출 시점 분석

**LobbyMap 진입 시:**
```
1. PostLogin (서버) → PlayerIndex = 0 할당
2. BeginPlay (클라이언트) → ServerRPC_SetPlayerNickname(qqq)
3. ServerRPC (서버) → SetPlayerInfo(qqq, 0)  ← NewIndex = 0 (정상)
```

**MainLevel_WP 전환 후:**
```
1. Seamless Travel → PlayerState 유지 (PlayerIndex = 0, 1)
2. PostLogin (서버) → PlayerIndex 이미 설정됨 (스킵)
3. OnPossess (클라이언트) → ServerRPC_SetPlayerNickname(qqq) 재호출
4. ServerRPC (서버) → SetPlayerInfo(qqq, PlayerArray.Num()-1)  ← ❌ 잘못된 계산
```

### 문제 3: PlayerArray.Num()의 불확실성

#### PlayerArray 순서 보장 안 됨
```
Seamless Travel 시:
  - Host의 PlayerArray 순서: [Host, Guest] 또는 [Guest, Host]?
  - Guest의 PlayerArray 순서: 항상 동일?
  - 네트워크 동기화 순서에 따라 변경 가능
```

#### 타이밍 이슈
```
ServerRPC_SetPlayerNickname 호출 시점:
  - Host: PlayerArray.Num() = 1 또는 2?
  - Guest: PlayerArray.Num() = 1 또는 2?
  - 서버에서 두 플레이어 모두 PostLogin 완료 전에 호출될 수 있음
```

---

## 💡 해결 방안

### 해결책 1: ServerRPC_SetPlayerNickname에서 PlayerIndex 계산 제거 (권장)

#### 수정 전
```cpp
void APlayerControl::ServerRPC_SetPlayerNickname_Implementation(const FString& Nickname)
{
    if (AYiSanPlayerState* YSPlayerState = GetPlayerState<AYiSanPlayerState>())
    {
        if (AGameStateBase* GS = GetWorld()->GetGameState())
        {
            int32 NewIndex = GS->PlayerArray.Num() - 1;  // ❌ 제거 필요
            YSPlayerState->SetPlayerInfo(Nickname, NewIndex);

            if (AYiSanPlayerListManager* PlayerListManager = ...)
            {
                PlayerListManager->UpdatePlayerListAndBroadcast();
            }
        }
    }
}
```

#### 수정 후
```cpp
void APlayerControl::ServerRPC_SetPlayerNickname_Implementation(const FString& Nickname)
{
    if (AYiSanPlayerState* YSPlayerState = GetPlayerState<AYiSanPlayerState>())
    {
        // PlayerIndex는 이미 PostLogin에서 할당되었음
        // SetPlayerInfo는 닉네임만 설정 (PlayerIndex는 건드리지 않음)
        YSPlayerState->SetPlayerInfo(Nickname, YSPlayerState->PlayerIndex);

        PRINTLOG(TEXT("[PlayerControl] ServerRPC_SetPlayerNickname - Nickname: %s, PlayerIndex: %d (유지)"),
            *Nickname, YSPlayerState->PlayerIndex);

        // PlayerListManager 업데이트 요청
        if (AYiSanPlayerListManager* PlayerListManager =
            Cast<AYiSanPlayerListManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AYiSanPlayerListManager::StaticClass())))
        {
            PlayerListManager->UpdatePlayerListAndBroadcast();
        }
    }
}
```

#### 장점
- ✅ PlayerIndex는 PostLogin에서 한 번만 할당
- ✅ Seamless Travel 시에도 PlayerIndex 유지
- ✅ PlayerArray.Num() 순서 의존성 제거
- ✅ 네트워크 타이밍 이슈 회피

### 해결책 2: SetPlayerInfo에서 PlayerIndex 매개변수 제거 (선택사항)

#### 현재 문제
```cpp
// SetPlayerInfo가 PlayerIndex를 받지만 사용하지 않음
void SetPlayerInfo(const FString& InName, int32 InIndex)
{
    Nickname = InName;
    // PlayerIndex = InIndex;  // 주석 처리됨
}
```

#### 개선안
```cpp
// Header
void SetPlayerNickname(const FString& InName);

// Implementation
void AYiSanPlayerState::SetPlayerNickname(const FString& InName)
{
    if (HasAuthority())
    {
        Nickname = InName;
        PRINTLOG(TEXT("[PlayerState] Nickname set to: %s (PlayerIndex: %d)"),
            *Nickname, PlayerIndex);
    }
}
```

#### RPC 호출 수정
```cpp
void APlayerControl::ServerRPC_SetPlayerNickname_Implementation(const FString& Nickname)
{
    if (AYiSanPlayerState* YSPlayerState = GetPlayerState<AYiSanPlayerState>())
    {
        YSPlayerState->SetPlayerNickname(Nickname);  // 간결해짐

        // PlayerListManager 업데이트
        // ...
    }
}
```

### 해결책 3: 중복 호출 방지 (선택사항)

#### 현재 문제
```
BeginPlay → ServerRPC_SetPlayerNickname
OnPossess → ServerRPC_SetPlayerNickname  (중복)
```

#### 개선안
```cpp
// Header
bool bNicknameSet = false;

// BeginPlay
void APlayerControl::BeginPlay()
{
    // ...

    if (!bNicknameSet && !Nickname.IsEmpty())
    {
        ServerRPC_SetPlayerNickname(Nickname);
        bNicknameSet = true;
    }
}

// OnPossess
void APlayerControl::OnPossess(APawn* InPawn)
{
    // ...

    if (!bNicknameSet && !Nickname.IsEmpty())
    {
        ServerRPC_SetPlayerNickname(Nickname);
        bNicknameSet = true;
    }
}
```

---

## 📝 다음 세션 작업 계획

### 우선순위 1: ServerRPC_SetPlayerNickname 수정 (필수)

#### 파일: `Source/YiSan/Character/Private/APlayerControl.cpp`
#### 위치: 206-222번 줄

```cpp
// ❌ 제거할 코드
int32 NewIndex = GS->PlayerArray.Num() - 1;
YSPlayerState->SetPlayerInfo(Nickname, NewIndex);

// ✅ 수정할 코드
YSPlayerState->SetPlayerInfo(Nickname, YSPlayerState->PlayerIndex);
PRINTLOG(TEXT("[PlayerControl] ServerRPC_SetPlayerNickname - Nickname: %s, PlayerIndex: %d (유지)"),
    *Nickname, YSPlayerState->PlayerIndex);
```

### 우선순위 2: 로그 추가 및 검증

#### 추가할 로그
```cpp
// APlayerControl::BeginPlay
PRINTLOG(TEXT("[PlayerControl] BeginPlay - PC=%s, Nickname=%s"),
    *GetName(), *Nickname);

// APlayerControl::OnPossess
PRINTLOG(TEXT("[PlayerControl] OnPossess - PC=%s, Pawn=%s, PlayerIndex=%d"),
    *GetName(), *GetNameSafe(InPawn),
    GetPlayerState<AYiSanPlayerState>() ? GetPlayerState<AYiSanPlayerState>()->PlayerIndex : -1);

// ServerRPC_SetPlayerNickname
PRINTLOG(TEXT("[PlayerControl] ServerRPC_SetPlayerNickname - PC=%s, Nickname=%s, PlayerIndex=%d"),
    *GetName(), *Nickname,
    GetPlayerState<AYiSanPlayerState>() ? GetPlayerState<AYiSanPlayerState>()->PlayerIndex : -1);
```

### 우선순위 3: LoadingTransitions Standalone 노출 문제 확인

#### 확인할 코드
- `Source/YiSan/Loading/Private/ULoadingTransitionManager.cpp`
- `Source/YiSan/Loading/Private/UYiSanLoading.cpp`
- `APlayerControl::ClientRPC_ShowLoadingTransition_Implementation`

#### 확인 사항
1. Standalone 모드에서 Widget이 생성되는지 확인
2. Widget의 Visibility 설정 확인
3. ZOrder 확인 (다른 UI에 가려지는지)
4. Client/Server RPC 호출 여부 확인

### 우선순위 4: 테스트 시나리오

#### 시나리오 1: LobbyMap 정상 접속
```
1. Host(qqq) 로비 입장
   - 예상: PlayerIndex = 0, Nickname = "qqq"

2. Guest(www) 조인
   - 예상: PlayerIndex = 1, Nickname = "www"

3. StartWidget UI 확인
   - Host 화면: qqq(녹색), www(흰색)
   - Guest 화면: qqq(흰색), www(녹색)
```

#### 시나리오 2: MainLevel_WP 전환
```
1. StartLevel → MainLevel_WP Seamless Travel

2. PostLogin 로그 확인
   - 예상: "PlayerIndex ALREADY SET: 0", "PlayerIndex ALREADY SET: 1"

3. ServerRPC_SetPlayerNickname 로그 확인
   - Host: "Nickname: qqq, PlayerIndex: 0 (유지)"
   - Guest: "Nickname: www, PlayerIndex: 1 (유지)"

4. PlayerWidget UI 확인
   - Host 화면: qqq(녹색), www(흰색)
   - Guest 화면: qqq(흰색), www(녹색)
```

#### 시나리오 3: 난입 테스트
```
1. MainLevel_WP에서 새 플레이어(zzz) 조인

2. PostLogin 로그 확인
   - 예상: "Assigned NEW PlayerIndex 2 to zzz"

3. UI 확인
   - zzz 화면: qqq(흰색), www(흰색), zzz(녹색)
```

### 우선순위 5: 선택적 개선 사항

#### 1. SetPlayerInfo → SetPlayerNickname 리팩토링
- PlayerIndex 매개변수 제거
- 함수 이름을 목적에 맞게 변경

#### 2. 중복 호출 방지 플래그 추가
- `bNicknameSet` 플래그로 중복 RPC 호출 방지

#### 3. Seamless Travel 검증 로그 추가
- BeginPlay, PostLogin, OnPossess 각 단계별 상세 로그

---

## 🔧 수정할 파일 요약

### 필수 수정
1. **APlayerControl.cpp** (206-222번 줄)
   - `ServerRPC_SetPlayerNickname_Implementation` 함수 수정
   - `PlayerArray.Num()-1` 계산 제거
   - `YSPlayerState->PlayerIndex` 사용

### 확인 필요
1. **ULoadingTransitionManager.cpp**
   - Standalone 모드에서 Widget 노출 문제 확인

2. **UYiSanLoading.cpp**
   - ClientTravel 시 로딩 화면 표시 로직 확인

### 선택적 개선
1. **AYiSanPlayerState.h/cpp**
   - `SetPlayerInfo` → `SetPlayerNickname` 리팩토링

2. **APlayerControl.h/cpp**
   - `bNicknameSet` 플래그 추가

---

## 📊 현재 상태

### ✅ 완료된 작업
- [x] PlayerIndex 할당 로직 분석 완료
- [x] 문제 원인 파악 완료
- [x] 해결 방안 설계 완료
- [x] 백업 문서 작성 완료

### 🔄 진행 중인 작업
- [ ] APlayerControl.cpp 수정
- [ ] LoadingTransitions 문제 확인
- [ ] 로그 추가 및 검증

### ⏳ 대기 중인 작업
- [ ] 전체 시나리오 테스트
- [ ] 선택적 리팩토링

---

## 📚 참고 문서

### 이전 작업 기록
- `Documents/DevLog/PlayerIndex_Fix_Report.md` - 2025-11-01 PlayerIndex 색상 표시 문제 해결 보고서

### 관련 코드
- `Source/YiSan/Environment/Private/ABaseGameMode.cpp` - PlayerIndex 할당 로직
- `Source/YiSan/Environment/Public/UYiSanGameInstance.h` - PlayerIndex 카운터 관리
- `Source/YiSan/Character/Private/APlayerControl.cpp` - 문제 발생 지점
- `Source/YiSan/Environment/Private/AYiSanPlayerState.cpp` - PlayerState 관리

### Unreal Engine 문서
- [Seamless Travel](https://docs.unrealengine.com/5.3/en-US/travelling-in-multiplayer-in-unreal-engine/)
- [PlayerState Replication](https://docs.unrealengine.com/5.3/en-US/player-state-in-unreal-engine/)
- [RPC](https://docs.unrealengine.com/5.3/en-US/rpcs-in-unreal-engine/)

---

## 💬 메모

### 핵심 인사이트
1. **PlayerIndex는 PostLogin에서 한 번만 할당해야 함**
   - BeginPlay, OnPossess에서 재할당 금지
   - Seamless Travel로 자동 유지됨

2. **PlayerArray.Num()은 신뢰할 수 없음**
   - 네트워크 순서 보장 안 됨
   - 타이밍 이슈 있음
   - 절대 인덱스 계산에 사용하지 말 것

3. **GameInstance는 맵 전환 시에도 유지**
   - NextPlayerIndex 카운터가 계속 증가
   - 난입 시에도 올바른 순서 보장

### 의문점
1. **왜 SetPlayerInfo에서 PlayerIndex가 주석 처리되었는가?**
   - 이전 작업에서 이미 문제를 인식했을 가능성
   - 주석 처리로 임시 방편 적용
   - 하지만 잘못된 계산 로직은 그대로 남아있음

2. **BeginPlay와 OnPossess 둘 다에서 ServerRPC를 호출하는 이유는?**
   - 리스폰 상황 대비?
   - Seamless Travel 대비?
   - 중복 호출 방지 로직 필요

---

---

## 🎯 세션 2: Seamless Travel PlayerState 소실 문제 해결 (2025-11-02)

**시작 시간:** 11:00 KST
**종료 시간:** 11:45 KST
**상태:** ✅ 해결 완료

### 🚨 새로운 문제 발견

#### 증상
```
StartLevel: Host PlayerIndex = 0 ✓
MainMap: Host PlayerIndex = -1 ✗ (새로운 PlayerState 생성됨!)
```

#### 로그 분석
```
[11:44:01] StartLevel PostLogin: PlayerState=BP_YiSanPlayerState_C_0, PlayerIndex=0
[11:44:06] MainMap HandleSeamlessTravelPlayer: PlayerState=BP_YiSanPlayerState_C_1, PlayerIndex=-1
```

**핵심 발견**: PlayerState 객체 이름이 `_C_0` → `_C_1`로 변경됨
- Seamless Travel이 작동하지 않고 새 PlayerState가 생성되고 있음
- Host는 MainMap에서 PostLogin이 호출되지 않음 (Seamless Travel 특성)
- 따라서 PlayerIndex가 할당되지 않음

### 🔍 근본 원인

#### 1. Seamless Travel PlayerState 재생성 문제
```cpp
// Lobby → StartLevel 이동 시
GetWorld()->ServerTravel("/Game/.../StartLevel?listen", true);
                                                          ^^^^
                                                          bAbsolute=true
```

- `bAbsolute=true`: PlayerState 새로 생성 (Absolute Travel)
- `bAbsolute=false`: PlayerState 유지 (Relative Travel)

**하지만**: Lobby는 로컬 게임이고 StartLevel은 새 세션이므로 Absolute가 맞음

#### 2. Host PostLogin 미호출
- Seamless Travel 시 Host는 기존 접속 유지
- PostLogin은 새로 접속하는 플레이어(Guest)만 호출됨
- Host는 `HandleSeamlessTravelPlayer`만 호출됨

#### 3. BeginPlay vs PostLogin 호출 순서
```
[11:30:22.489] PostLogin - PlayerIndex 0 할당
[11:30:22.492] BeginPlay - NextPlayerIndex = 0 리셋  ← 문제!
```

BeginPlay가 PostLogin 이후에 호출되어 NextPlayerIndex를 리셋함

### 💡 최종 해결책

#### 1. InitGame으로 초기화 이동
```cpp
// AYiSanGameMode.h
virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

// AYiSanGameMode.cpp
void AYiSanGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // InitGame은 PostLogin보다 먼저 호출됨
    if (MapName.Contains(TEXT("StartLevel")))
    {
        AYisanGameState::NextPlayerIndex = 0;
        PRINTLOG(TEXT("[GameMode] InitGame - Reset NextPlayerIndex to 0 (StartLevel)"));
    }
    else
    {
        PRINTLOG(TEXT("[GameMode] InitGame - Skipping NextPlayerIndex reset (Map: %s)"), *MapName);
    }
}
```

**호출 순서**: InitGame → PostLogin → BeginPlay ✓

#### 2. HandleSeamlessTravelPlayer에서 PlayerIndex 할당
```cpp
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
```

#### 3. PostLogin에서 중복 할당 방지 (이미 적용됨)
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

#### 4. PlayerState 기본값 -1 초기화
```cpp
// AYiSanPlayerState.cpp
AYiSanPlayerState::AYiSanPlayerState()
    : PlayerIndex(-1)  // 명시적 초기화
{
}
```

### 📊 수정된 파일 목록

#### 필수 수정
1. **AYiSanGameMode.h** (19줄)
   - `InitGame` 함수 선언 추가
   - `HandleSeamlessTravelPlayer` 함수 선언 추가

2. **AYiSanGameMode.cpp** (13-28줄, 55-89줄)
   - `InitGame` 구현: StartLevel에서만 NextPlayerIndex 초기화
   - `HandleSeamlessTravelPlayer` 구현: Seamless Travel 플레이어 PlayerIndex 할당
   - `PostLogin` 수정: 중복 할당 방지 로직

3. **AYiSanPlayerState.cpp** (7-9줄)
   - 생성자에서 `PlayerIndex(-1)` 초기화

4. **ALobbyGameMode.cpp** (28-44줄)
   - PostLogin에서 PlayerIndex 할당 제거 (Lobby는 로컬 전용)
   - GameLogging.h include 추가

5. **AYisanGameState.h/cpp** (56줄, 17줄)
   - `static int32 NextPlayerIndex` 추가
   - static 변수 정의

#### 제거된 로직
- ALobbyGameMode의 PlayerIndex 할당 (Lobby는 무시)
- 각 GameMode의 local static NextPlayerIndex (GameState로 통합)

### 🎯 동작 흐름

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
   - NextPlayerIndex = 0 초기화 (StartLevel이므로)
2. AYiSanGameMode::PostLogin (Host)
   - PlayerIndex < 0 → 0 할당 ✓
3. AYiSanGameMode::PostLogin (Guest 접속 시)
   - PlayerIndex < 0 → 1 할당 ✓
```

#### MainMap (Seamless Travel)
```
1. AYiSanGameMode::InitGame
   - NextPlayerIndex 초기화 건너뜀 (StartLevel이 아니므로)
   - NextPlayerIndex = 1 유지 (Host=0 이미 할당됨)
2. AYiSanGameMode::HandleSeamlessTravelPlayer (Host)
   - PlayerState 새로 생성됨 (PlayerIndex = -1)
   - PlayerIndex < 0 → 0 할당 ✓
3. AYiSanGameMode::PostLogin (Guest)
   - PlayerIndex < 0 → 1 할당 ✓
```

### ✅ 검증 결과

#### 예상 로그
```
[StartLevel InitGame] Reset NextPlayerIndex to 0 (StartLevel)
[StartLevel PostLogin] Assigned PlayerIndex 0 to Host
[StartLevel PostLogin] Assigned PlayerIndex 1 to Guest

[MainMap InitGame] Skipping NextPlayerIndex reset (Map: MainMap_WP)
[MainMap HandleSeamlessTravelPlayer] PlayerIndex=-1 (before)
[MainMap HandleSeamlessTravelPlayer] Assigned PlayerIndex 0
[MainMap PostLogin] Assigned PlayerIndex 1 to Guest

[UpdatePlayerList] Current players: 0:host, 1:guest ✓
```

### 🔧 핵심 개선 사항

#### 1. 생명주기 이해
```
InitGame (제일 먼저)
  → PostLogin (새 접속자)
  → HandleSeamlessTravelPlayer (Seamless Travel 플레이어)
  → BeginPlay (제일 나중)
```

#### 2. Seamless Travel 특성
- PlayerState는 **새로 생성**될 수 있음 (Unreal Engine 내부 구현)
- Host는 PostLogin이 호출되지 않음
- `HandleSeamlessTravelPlayer`에서 PlayerIndex 재할당 필요

#### 3. static 변수 관리
- GameState에 통합하여 일관성 확보
- 레벨 전환 시에도 프로세스 전역으로 유지됨

### 📝 남은 작업

#### 우선순위 1: 빌드 및 테스트
- [x] InitGame 구현
- [x] HandleSeamlessTravelPlayer 구현
- [x] PlayerState 기본값 초기화
- [ ] 빌드 테스트
- [ ] 실제 게임 시나리오 검증

#### 우선순위 2: 이전 문제 해결 (세션 1)
- [ ] ServerRPC_SetPlayerNickname 수정
  - PlayerArray.Num()-1 계산 제거
  - YSPlayerState->PlayerIndex 사용

### 💡 학습 내용

#### Seamless Travel의 함정
1. PlayerState가 항상 유지되는 것은 아님
2. Host는 PostLogin이 호출되지 않음
3. HandleSeamlessTravelPlayer에서 처리 필요

#### GameMode 생명주기
1. InitGame이 제일 먼저 호출됨
2. 초기화는 InitGame에서 해야 타이밍 안전

#### Static 변수의 위치
1. 여러 GameMode가 공유해야 하면 GameState
2. 프로세스 전역이므로 레벨 전환에도 유지

---

**문서 버전:** 2.0
**작성자:** Claude Code
**다음 세션:** 빌드 테스트 및 세션 1 문제 해결
**최종 수정:** 2025-11-02 11:45 KST
