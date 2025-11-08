# PlayerIndex 관리 - GameState 리팩토링 (2025-11-02)

**작성일:** 2025-11-02
**상태:** ✅ 완료

---

## 목차
1. [문제 상황](#문제-상황)
2. [원인 분석](#원인-분석)
3. [해결 방안](#해결-방안)
4. [최종 구현](#최종-구현)
5. [수정 파일 목록](#수정-파일-목록)
6. [테스트 가이드](#테스트-가이드)

---

## 문제 상황

### 증상

```
로그 분석 결과:
  Player 0: 2:qqq
  Player 1: 3:www
  LocalPlayerIndex = 3

예상: PlayerIndex 0, 1
실제: PlayerIndex 2, 3
```

**Guest(www) 유저의 정보가 UI에 잘못 표시되는 문제 발생**

### 근본 원인

1. **GameInstance의 NextPlayerIndex가 PIE 세션 간에 유지됨**
   - 이전 세션에서 NextPlayerIndex = 2
   - 새 세션 시작 시 ResetPlayerIndex() 미호출
   - 결과: 0, 1 대신 2, 3이 할당됨

2. **ResetPlayerIndex() 호출 조건 문제**
   ```cpp
   if (GS->PlayerArray.Num() == 0)  // BeginPlay 타이밍에 이미 플레이어 존재
   {
       GI->ResetPlayerIndex();  // 호출되지 않음
   }
   ```

3. **복제(Replication) 구조 문제**
   - GameInstance는 복제되지 않음
   - 서버의 NextPlayerIndex만 증가
   - 클라이언트는 이 값을 알 수 없음
   - PlayerState의 PlayerIndex는 복제되지만, 할당 로직이 불안정

---

## 원인 분석

### 기존 구조의 문제점

```
GameInstance (복제 안 됨)
  └─ NextPlayerIndex (서버 전용, 클라 모름)
      └─ BaseGameMode::PostLogin
          └─ PlayerState->PlayerIndex 할당
              └─ 복제됨 (하지만 소스가 불안정)
```

**문제:**
- GameInstance는 맵 전환 시 유지되지만, PIE 세션 간에도 유지됨
- ResetPlayerIndex() 타이밍 이슈
- 복제 안 되는 GameInstance에 의존

### 요구사항

```
서버가 순번 매기고, 클라는 받아만 쓰는 구조

- 서버 전용으로 NextPlayerIndex 관리
- PlayerState가 서버에서 인덱스 배정 → Replicated
- 클라이언트는 복제된 값만 사용
- 맵 전환 시에도 순번 유지 (Seamless Travel 전제)
```

---

## 해결 방안

### 전형 패턴 적용

| 역할 | 클래스 | 기능 |
|------|--------|------|
| 순번 보관 | **AYisanGameState** | NextPlayerIndex 증가, 서버만 변경 |
| 순번 적용 | **AYiSanPlayerState** | PlayerIndex 복제, OnRep로 UI 연결 가능 |
| 순번 부여 | **ABaseGameMode** | 플레이어 로그인 시 인덱스 지정 |

### 왜 GameState인가?

1. **GameState는 항상 복제됨**
   - 서버/클라 모두 존재
   - Seamless Travel 시 유지됨

2. **GameMode는 서버 전용**
   - PostLogin에서 GameState 접근 가능
   - GameState->GetNextPlayerIndex() 호출

3. **GameInstance 대비 장점**
   - PIE 세션 간 격리 (새 게임 = 새 GameState)
   - 복제 구조와 자연스럽게 통합
   - 맵 전환 시 자동 유지 (Seamless Travel)

---

## 최종 구현

### 1. AYisanGameState.h

```cpp
class YISAN_API AYisanGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    /** @brief 다음 PlayerIndex를 반환하고 증가시킵니다. 서버 전용입니다. */
    int32 GetNextPlayerIndex();

    /** @brief PlayerIndex 카운터를 초기화합니다. 서버 전용입니다. */
    void ResetPlayerIndex();

private:
    /** @brief 다음에 할당할 PlayerIndex (서버에서만 증가, 복제 불필요) */
    int32 NextPlayerIndex = 0;
};
```

### 2. AYisanGameState.cpp

```cpp
int32 AYisanGameState::GetNextPlayerIndex()
{
    if (!HasAuthority())
    {
        PRINTLOG(TEXT("[GameState] ERROR: GetNextPlayerIndex called on client!"));
        return -1;
    }

    int32 CurrentIndex = NextPlayerIndex;
    NextPlayerIndex++;

    PRINTLOG(TEXT("[GameState] GetNextPlayerIndex - Returning: %d, Next will be: %d"),
        CurrentIndex, NextPlayerIndex);

    return CurrentIndex;
}

void AYisanGameState::ResetPlayerIndex()
{
    if (!HasAuthority())
    {
        PRINTLOG(TEXT("[GameState] ERROR: ResetPlayerIndex called on client!"));
        return;
    }

    NextPlayerIndex = 0;
    PRINTLOG(TEXT("[GameState] ResetPlayerIndex - NextPlayerIndex reset to 0"));
}
```

### 3. ABaseGameMode.cpp

```cpp
void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!NewPlayer) return;

    AYiSanPlayerState* PS = NewPlayer->GetPlayerState<AYiSanPlayerState>();
    if (!PS) return;

    PRINTLOG(TEXT("[BaseGameMode] PostLogin - PC=%s, PlayerState=%s, Current PlayerIndex=%d"),
        *GetNameSafe(NewPlayer), *GetNameSafe(PS), PS->PlayerIndex);

    // PlayerIndex가 아직 할당되지 않았으면 할당
    if (PS->PlayerIndex == -1)
    {
        if (AYisanGameState* GS = GetGameState<AYisanGameState>())
        {
            PS->PlayerIndex = GS->GetNextPlayerIndex();
            PRINTLOG(TEXT("[BaseGameMode] Assigned NEW PlayerIndex %d to %s"),
                PS->PlayerIndex, *PS->GetPlayerName());
        }
        else
        {
            PRINTLOG(TEXT("[BaseGameMode] ERROR: GameState not found! Cannot assign PlayerIndex"));
        }
    }
    else
    {
        PRINTLOG(TEXT("[BaseGameMode] PlayerIndex ALREADY SET: %d (Seamless Travel 성공)"),
            PS->PlayerIndex);
    }
}
```

### 4. ALobbyGameMode.cpp

```cpp
void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    FString CurrentMapName = GetWorld()->GetMapName();
    PRINTLOG(TEXT("============================================"));
    PRINTLOG(TEXT("[LobbyGameMode] BeginPlay CALLED"));
    PRINTLOG(TEXT("  Current Map: %s"), *CurrentMapName);

    // LobbyMap에서만 PlayerIndex 초기화
    bool bIsLobbyMap = CurrentMapName.Contains(TEXT("LobbyMap")) ||
                       CurrentMapName.Contains(TEXT("StartLevel")) ||
                       CurrentMapName.Contains(TEXT("Lobby"));

    if (bIsLobbyMap)
    {
        PRINTLOG(TEXT("  This is Lobby Map - Resetting PlayerIndex"));
        if (AYisanGameState* GS = GetGameState<AYisanGameState>())
        {
            GS->ResetPlayerIndex();
            PRINTLOG(TEXT("[LobbyGameMode] PlayerIndex counter reset to 0"));
        }
        else
        {
            PRINTLOG(TEXT("[LobbyGameMode] ERROR: GameState not found!"));
        }
    }
    else
    {
        PRINTLOG(TEXT("  WARNING: LobbyGameMode is running on non-lobby map!"));
        PRINTLOG(TEXT("  WARNING: NOT resetting PlayerIndex to preserve Seamless Travel"));
        PRINTLOG(TEXT("  Current Map: %s (expected: LobbyMap)"), *CurrentMapName);
    }
    PRINTLOG(TEXT("============================================"));
}
```

---

## 아키텍처 다이어그램

```
┌─────────────────────────────────────────────────────────────┐
│                   AYisanGameState                           │
│  - NextPlayerIndex: int32 (서버 전용, 복제 불필요)           │
│  + GetNextPlayerIndex(): int32                              │
│  + ResetPlayerIndex(): void                                 │
│                                                             │
│  [Seamless Travel 시 자동 유지]                             │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ 사용
                              ▼
        ┌─────────────────────────────────────────┐
        │                                         │
        ▼                                         ▼
┌──────────────────┐                   ┌──────────────────┐
│ ALobbyGameMode   │                   │ AYiSanGameMode   │
│  (서버 전용)     │  Seamless Travel  │  (서버 전용)     │
│                  │──────────────────▶│                  │
│ BeginPlay():     │                   │ BeginPlay():     │
│   GS->Reset()    │                   │   (초기화 없음)  │
│                  │                   │                  │
│ PostLogin():     │                   │ PostLogin():     │
│   Base 호출      │                   │   Base 호출      │
└──────────────────┘                   └──────────────────┘
        │                                         │
        │ 모두 Base로 위임                         │
        ▼                                         ▼
┌─────────────────────────────────────────────────────────────┐
│                   ABaseGameMode                             │
│  PostLogin():                                               │
│    if (PS->PlayerIndex == -1)                               │
│      PS->PlayerIndex = GS->GetNextPlayerIndex()             │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ 할당
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  AYiSanPlayerState                          │
│  UPROPERTY(Replicated)                                      │
│  int32 PlayerIndex = -1                                     │
│                                                             │
│  [서버에서 할당 → 자동으로 클라이언트에 복제됨]              │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ 복제
                              ▼
                     클라이언트 UI 업데이트
```

---

## 동작 흐름

### 로비 입장 시

```
1. LobbyMap 로드
   └─ ALobbyGameMode::BeginPlay()
      └─ GS->ResetPlayerIndex()
         └─ NextPlayerIndex = 0

2. Host(qqq) 접속
   └─ ABaseGameMode::PostLogin(Host)
      └─ PS->PlayerIndex == -1 (초기값)
      └─ PS->PlayerIndex = GS->GetNextPlayerIndex()
         └─ NextPlayerIndex: 0 → 1
         └─ Host.PlayerIndex = 0 ✅

3. Guest(www) 접속
   └─ ABaseGameMode::PostLogin(Guest)
      └─ PS->PlayerIndex == -1 (초기값)
      └─ PS->PlayerIndex = GS->GetNextPlayerIndex()
         └─ NextPlayerIndex: 1 → 2
         └─ Guest.PlayerIndex = 1 ✅
```

### 메인 맵 전환 시 (Seamless Travel)

```
1. StartLevel → MainLevel_WP
   └─ Seamless Travel 실행
      └─ GameState 유지 (NextPlayerIndex = 2)
      └─ PlayerState 유지 (Host=0, Guest=1)

2. AYiSanGameMode::BeginPlay()
   └─ ResetPlayerIndex() 호출 안 함 ✅

3. Host PostLogin
   └─ PS->PlayerIndex == 0 (유지됨)
   └─ "PlayerIndex ALREADY SET: 0" 로그
   └─ 할당 스킵 ✅

4. Guest PostLogin
   └─ PS->PlayerIndex == 1 (유지됨)
   └─ "PlayerIndex ALREADY SET: 1" 로그
   └─ 할당 스킵 ✅
```

### 난입 시

```
메인맵에서 새 플레이어(zzz) 조인:

1. ABaseGameMode::PostLogin(zzz)
   └─ PS->PlayerIndex == -1 (신규)
   └─ PS->PlayerIndex = GS->GetNextPlayerIndex()
      └─ NextPlayerIndex: 2 → 3
      └─ zzz.PlayerIndex = 2 ✅

결과:
  Host: 0
  Guest: 1
  난입자: 2
```

---

## 수정 파일 목록

### 수정된 파일

1. **Source/YiSan/Environment/Public/AYisanGameState.h**
   - `GetNextPlayerIndex()` 함수 추가
   - `ResetPlayerIndex()` 함수 추가
   - `NextPlayerIndex` 멤버 변수 추가

2. **Source/YiSan/Environment/Private/AYisanGameState.cpp**
   - `GetNextPlayerIndex()` 구현
   - `ResetPlayerIndex()` 구현

3. **Source/YiSan/Environment/Private/ABaseGameMode.cpp**
   - include: `UYiSanGameInstance.h` → `AYisanGameState.h`
   - PostLogin: `GI->GetNextPlayerIndex()` → `GS->GetNextPlayerIndex()`

4. **Source/YiSan/Environment/Private/ALobbyGameMode.cpp**
   - include: `UYiSanGameInstance.h` → `AYisanGameState.h`
   - BeginPlay: `GI->ResetPlayerIndex()` → `GS->ResetPlayerIndex()`
   - PlayerArray.Num() 조건 제거 (무조건 리셋)

### 삭제 가능한 코드 (선택사항)

**Source/YiSan/Environment/Public/UYiSanGameInstance.h**
```cpp
// 더 이상 사용하지 않음 (삭제 가능)
int32 GetNextPlayerIndex();
void ResetPlayerIndex();
int32 NextPlayerIndex = 0;
```

**Source/YiSan/Environment/Private/UYiSanGameInstance.cpp**
```cpp
// 더 이상 사용하지 않음 (삭제 가능)
int32 UYiSanGameInstance::GetNextPlayerIndex() { ... }
void UYiSanGameInstance::ResetPlayerIndex() { ... }
```

**주의:** GameInstance는 다른 용도로 사용 중일 수 있으므로, PlayerIndex 관련 코드만 제거하고 클래스 자체는 유지합니다.

---

## 테스트 가이드

### 시나리오 1: 로비 정상 접속

```
1. PIE 또는 Standalone으로 LobbyMap 시작
2. 로그 확인:
   [LobbyGameMode] PlayerIndex counter reset to 0

3. Host 접속 후 로그:
   [GameState] GetNextPlayerIndex - Returning: 0, Next will be: 1
   [BaseGameMode] Assigned NEW PlayerIndex 0 to qqq

4. Guest 접속 후 로그:
   [GameState] GetNextPlayerIndex - Returning: 1, Next will be: 2
   [BaseGameMode] Assigned NEW PlayerIndex 1 to www

5. UI 확인:
   Host 화면: qqq(녹색), www(흰색)
   Guest 화면: qqq(흰색), www(녹색)
```

### 시나리오 2: 메인 맵 전환

```
1. StartLevel → MainLevel_WP Seamless Travel
2. 로그 확인:
   [BaseGameMode] PlayerIndex ALREADY SET: 0 (Seamless Travel 성공)
   [BaseGameMode] PlayerIndex ALREADY SET: 1 (Seamless Travel 성공)

3. UI 확인:
   Host 화면: qqq(녹색), www(흰색)
   Guest 화면: qqq(흰색), www(녹색)
```

### 시나리오 3: PIE 재시작 테스트

```
1. 첫 번째 PIE 세션 실행
   Host: PlayerIndex = 0
   Guest: PlayerIndex = 1

2. PIE 종료

3. 두 번째 PIE 세션 실행
   Host: PlayerIndex = 0 (다시 0부터 시작) ✅
   Guest: PlayerIndex = 1 ✅

검증: 이전 세션의 NextPlayerIndex=2가 유지되지 않음
```

### 시나리오 4: 난입 테스트

```
1. MainLevel_WP에서 3번째 플레이어 조인
2. 로그 확인:
   [GameState] GetNextPlayerIndex - Returning: 2, Next will be: 3
   [BaseGameMode] Assigned NEW PlayerIndex 2 to zzz

3. UI 확인:
   zzz 화면: qqq(흰색), www(흰색), zzz(녹색)
```

---

## 핵심 학습 사항

### 1. GameState의 활용

```
GameState는 멀티플레이어에서 서버 권한 데이터의 중앙 저장소

장점:
- 항상 복제됨 (서버/클라 모두 존재)
- Seamless Travel 시 자동 유지
- PIE 세션마다 새로 생성 (격리)
- GameMode에서 쉽게 접근 가능
```

### 2. 복제 구조 설계

```
복제가 필요한 데이터: PlayerState.PlayerIndex (각 플레이어마다)
복제 불필요한 데이터: GameState.NextPlayerIndex (서버 내부 카운터)

원칙:
- 서버에서만 증가하는 카운터는 복제 불필요
- 클라이언트가 읽어야 하는 최종 값만 복제
```

### 3. GameInstance vs GameState

| | GameInstance | GameState |
|---|---|---|
| 복제 | ❌ 안 됨 | ✅ 됨 |
| Seamless Travel | ✅ 유지 | ✅ 유지 |
| PIE 세션 격리 | ❌ (유지될 수 있음) | ✅ (항상 새로 생성) |
| 서버/클라 존재 | 둘 다 | 둘 다 |
| 용도 | 세션, 설정 관리 | 게임 상태, 플레이어 관리 |

**결론:** PlayerIndex 같은 게임 내 순번 관리는 **GameState가 적합**

---

## 향후 개선 사항

### 1. PlayerIndex 재사용 (선택사항)

현재는 플레이어가 나가도 인덱스가 계속 증가합니다. 필요 시 재사용 로직 추가:

```cpp
class AYisanGameState
{
    TArray<int32> AvailableIndices;

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

class ABaseGameMode
{
    void Logout(AController* Exiting) override
    {
        Super::Logout(Exiting);

        if (AYiSanPlayerState* PS = Exiting->GetPlayerState<AYiSanPlayerState>())
        {
            if (AYisanGameState* GS = GetGameState<AYisanGameState>())
            {
                GS->ReleasePlayerIndex(PS->PlayerIndex);
            }
        }
    }
};
```

### 2. 최대 플레이어 수 제한

```cpp
int32 AYisanGameState::GetNextPlayerIndex()
{
    if (NextPlayerIndex >= MaxPlayers)
    {
        PRINTLOG(TEXT("[GameState] ERROR: Max player limit reached!"));
        return -1;
    }
    return NextPlayerIndex++;
}
```

---

## 참고 자료

### Unreal Engine 공식 문서
- [GameState](https://docs.unrealengine.com/5.3/en-US/game-state-in-unreal-engine/)
- [GameMode](https://docs.unrealengine.com/5.3/en-US/game-mode-in-unreal-engine/)
- [Seamless Travel](https://docs.unrealengine.com/5.3/en-US/travelling-in-multiplayer-in-unreal-engine/)
- [Replication](https://docs.unrealengine.com/5.3/en-US/replication-in-unreal-engine/)

### 관련 이슈
- `Documents/DevLog/PlayerIndex_Fix_Report.md` - 초기 PlayerIndex 할당 문제 해결
- `Documents/DevLog/PlayerIndex_Issue_2025-11-02.md` - PlayerIndex 2,3 문제 분석

---

**문서 버전:** 1.0
**작성자:** Claude Code
**최종 수정:** 2025-11-02 02:00 KST
