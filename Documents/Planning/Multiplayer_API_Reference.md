# YiSan 멀티플레이 API 레퍼런스

**작성일:** 2025-10-20
**버전:** 1.0
**대상:** C++ 개발자, Blueprint 스크립터

---

## 📑 목차

1. [UYiSanGameInstance](#1-uyisangameinstance)
2. [ALobbyGameMode](#2-alobbygamemode)
3. [AYiSanGameMode](#3-ayisangamemode)
4. [APlayerControl](#4-aplayercontrol)
5. [ULobbyWidget](#5-ulobbywidget)
6. [UBroadcastManager](#6-ubroadcastmanager)

---

## 1. UYiSanGameInstance

**경로:** `Source/YiSan/Loading/Public/YiSanGameInstance.h`

게임 인스턴스 클래스로, 세션 생명주기를 관리합니다.

### 1.1 Public Functions

#### `HostGame`

```cpp
UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
bool HostGame(const FString& MapName = TEXT("MainMap_WP"), int32 MaxPlayers = 4);
```

**설명:** 호스트로 게임 세션을 생성합니다.

**매개변수:**
- `MapName` (FString): 호스팅할 맵 이름
  - 기본값: `"MainMap_WP"`
  - 예시: `"MainMap_WP"`, `"TestMap"`
- `MaxPlayers` (int32): 최대 플레이어 수
  - 기본값: `4`
  - 범위: 1~16 권장

**반환값:**
- `bool`: 세션 생성 성공 시 `true`, 실패 시 `false`

**동작:**
1. TravelURL 생성: `/Game/YiSan/Maps/{MapName}?listen?MaxPlayers={MaxPlayers}`
2. `World->ServerTravel(TravelURL)` 호출
3. 리슨 서버 활성화
4. `bIsHost = true`, `bIsInSession = true` 설정
5. `UBroadcastManager::SendSessionHost(MapName)` 호출

**사용 예시 (C++):**
```cpp
if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
{
    bool bSuccess = GI->HostGame(TEXT("MainMap_WP"), 4);
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("세션 생성 성공!"));
    }
}
```

**사용 예시 (Blueprint):**
```
Get Game Instance
  → Cast to YiSanGameInstance
  → Host Game (Map Name: "MainMap_WP", Max Players: 4)
  → Branch (Success)
```

**오류 처리:**
- World가 null인 경우: `false` 반환, 로그 출력

---

#### `JoinGame`

```cpp
UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
bool JoinGame(const FString& Address, int32 Port = 7777);
```

**설명:** 기존 게임 세션에 클라이언트로 참가합니다.

**매개변수:**
- `Address` (FString): 서버 IP 주소 또는 MagicDNS 도메인
  - 예시: `"192.168.0.10"`, `"host-pc"`, `"100.64.0.5"`
  - 빈 문자열 불가
- `Port` (int32): 서버 포트
  - 기본값: `7777`
  - 범위: 1024~65535

**반환값:**
- `bool`: 접속 시도 성공 시 `true`, 실패 시 `false`

**동작:**
1. Address 유효성 검사
2. TravelURL 생성: `{Address}:{Port}`
3. `PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute)` 호출
4. `bIsHost = false`, `bIsInSession = true` 설정
5. `UBroadcastManager::SendSessionJoin(Address, Port)` 호출

**사용 예시 (C++):**
```cpp
if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
{
    bool bSuccess = GI->JoinGame(TEXT("192.168.0.10"), 7777);
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("서버 접속 시도 중..."));
    }
}
```

**사용 예시 (Blueprint):**
```
Get Game Instance
  → Cast to YiSanGameInstance
  → Join Game (Address: "192.168.0.10", Port: 7777)
  → Branch (Success)
```

**오류 처리:**
- World가 null인 경우: `false` 반환
- Address가 빈 문자열인 경우: `false` 반환

---

#### `DisconnectFromSession`

```cpp
UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
void DisconnectFromSession();
```

**설명:** 현재 세션에서 연결을 끊고 로비로 돌아갑니다.

**매개변수:** 없음

**반환값:** 없음

**동작:**
1. `bIsInSession` 체크 (이미 연결 해제 상태면 종료)
2. `UGameplayStatics::OpenLevel(this, "LobbyMap", false)` 호출
3. `bIsHost = false`, `bIsInSession = false` 초기화
4. `UBroadcastManager::SendSessionDisconnect()` 호출

**사용 예시 (C++):**
```cpp
if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
{
    GI->DisconnectFromSession();
}
```

**사용 예시 (Blueprint):**
```
Get Game Instance
  → Cast to YiSanGameInstance
  → Disconnect From Session
```

---

#### `IsHost`

```cpp
UFUNCTION(BlueprintPure, Category="Multiplayer|Session")
bool IsHost() const;
```

**설명:** 현재 호스트 상태인지 확인합니다.

**매개변수:** 없음

**반환값:**
- `bool`: 호스트인 경우 `true`, 아니면 `false`

**사용 예시 (C++):**
```cpp
if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
{
    if (GI->IsHost())
    {
        // 호스트 전용 UI 표시
    }
}
```

**사용 예시 (Blueprint):**
```
Get Game Instance
  → Cast to YiSanGameInstance
  → Is Host
  → Branch
```

---

#### `IsInSession`

```cpp
UFUNCTION(BlueprintPure, Category="Multiplayer|Session")
bool IsInSession() const;
```

**설명:** 현재 멀티플레이 세션에 연결되어 있는지 확인합니다.

**매개변수:** 없음

**반환값:**
- `bool`: 세션에 연결된 경우 `true`, 아니면 `false`

**사용 예시 (C++):**
```cpp
if (UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance()))
{
    if (GI->IsInSession())
    {
        // Disconnect 버튼 활성화
    }
}
```

---

## 2. ALobbyGameMode

**경로:** `Source/YiSan/Environment/Public/ALobbyGameMode.h`

로비 맵 전용 게임 모드입니다.

### 2.1 Public Properties

#### `bAllowJoinInProgress`

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby")
bool bAllowJoinInProgress = true;
```

**설명:** 게임 진행 중 난입 허용 여부

**기본값:** `true` (항상 허용)

---

#### `MaxPlayers`

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby")
int32 MaxPlayers = 4;
```

**설명:** 로비 최대 플레이어 수

**기본값:** `4`

**Blueprint에서 수정 가능**

---

### 2.2 Public Functions

#### `GetCurrentPlayerCount`

```cpp
UFUNCTION(BlueprintPure, Category="Lobby")
int32 GetCurrentPlayerCount() const;
```

**설명:** 현재 로비에 있는 플레이어 수를 반환합니다.

**반환값:**
- `int32`: 현재 플레이어 수

**사용 예시 (Blueprint):**
```
Get Game Mode
  → Cast to LobbyGameMode
  → Get Current Player Count
  → Print String
```

---

#### `IsLobbyFull`

```cpp
UFUNCTION(BlueprintPure, Category="Lobby")
bool IsLobbyFull() const;
```

**설명:** 로비가 가득 찼는지 확인합니다.

**반환값:**
- `bool`: 로비 풀 여부

**사용 예시 (Blueprint):**
```
Get Game Mode
  → Cast to LobbyGameMode
  → Is Lobby Full
  → Branch (If Full: Show "Lobby Full" Message)
```

---

### 2.3 Protected Functions (Override)

#### `PostLogin`

```cpp
virtual void PostLogin(APlayerController* NewPlayer) override;
```

**설명:** 플레이어가 로그인할 때 호출됩니다.

**동작:**
1. `CurrentPlayerCount++`
2. Pawn 생성 보장 (`SpawnDefaultPawnFor`)
3. 로그 출력

---

#### `Logout`

```cpp
virtual void Logout(AController* Exiting) override;
```

**설명:** 플레이어가 로그아웃할 때 호출됩니다.

**동작:**
1. `CurrentPlayerCount--` (0 이하로 가지 않음)
2. 로그 출력

---

## 3. AYiSanGameMode

**경로:** `Source/YiSan/Environment/Public/AYiSanGameMode.h`

메인 게임 로직을 담당하는 게임 모드입니다.

### 3.1 Public Functions

#### `StartTour`

```cpp
UFUNCTION(BlueprintCallable, Category="Tour")
void StartTour();
```

**설명:** 투어를 시작합니다 (Host만 호출 가능).

**권한:** `HasAuthority()` 필요

**동작:**
1. GameState의 `StartGlobalTour()` 호출
2. 모든 클라이언트에 복제됨

**사용 예시 (Blueprint):**
```
Get Game Mode
  → Cast to YiSanGameMode
  → Start Tour
```

---

#### `SetTourState`

```cpp
void SetTourState(EDasanState InState);
```

**설명:** 투어 상태를 변경합니다.

**매개변수:**
- `InState` (EDasanState): 새로운 투어 상태
  - `Tour`: 이동 중
  - `Explain`: 설명 중
  - `Answer`: 질문 답변 중

**권한:** `HasAuthority()` 필요

**동작:**
1. `DasanNPC->ServerRPC_SetDasanState(InState)` 호출
2. 모든 클라이언트에 복제됨

---

#### `PostLogin`

```cpp
virtual void PostLogin(APlayerController* NewPlayer) override;
```

**설명:** 플레이어가 로그인할 때 호출됩니다 (난입 플레이어 포함).

**동작:**
1. Pawn 생성 보장
2. DasanNPC 찾기 및 GameState 설정
3. 복제된 상태를 난입 플레이어에게 전송

---

## 4. APlayerControl

**경로:** `Source/LatteLibrary/Character/Public/APlayerControl.h`

플레이어 컨트롤러 클래스로, UI와 GameInstance를 연결합니다.

### 4.1 Multiplayer Session Functions

#### `HostSession`

```cpp
UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
void HostSession(const FString& MapName = TEXT("MainMap_WP"), int32 MaxPlayers = 4);
```

**설명:** UI 위젯에서 호출하는 Host 함수입니다.

**매개변수:**
- `MapName` (FString): 호스팅할 맵 이름
- `MaxPlayers` (int32): 최대 플레이어 수

**동작:**
1. GameInstance 가져오기
2. `UYiSanGameInstance::HostGame()` 호출

**사용 예시 (Blueprint Widget):**
```
Button OnClicked
  → Get Owning Player
  → Cast to PlayerControl
  → Host Session (Map Name: "MainMap_WP", Max Players: 4)
```

---

#### `JoinSession`

```cpp
UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
void JoinSession(const FString& Address, int32 Port = 7777);
```

**설명:** UI 위젯에서 호출하는 Join 함수입니다.

**매개변수:**
- `Address` (FString): 서버 IP 주소
- `Port` (int32): 서버 포트

**동작:**
1. GameInstance 가져오기
2. `UYiSanGameInstance::JoinGame()` 호출

**사용 예시 (Blueprint Widget):**
```
Button OnClicked
  → Get Text (from IP TextBox)
  → Get Owning Player
  → Cast to PlayerControl
  → Join Session (Address: IP Text, Port: 7777)
```

---

#### `DisconnectSession`

```cpp
UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
void DisconnectSession();
```

**설명:** UI 위젯에서 호출하는 Disconnect 함수입니다.

**동작:**
1. GameInstance 가져오기
2. `UYiSanGameInstance::DisconnectFromSession()` 호출

**사용 예시 (Blueprint Widget):**
```
Button OnClicked
  → Get Owning Player
  → Cast to PlayerControl
  → Disconnect Session
```

---

## 5. ULobbyWidget

**경로:** `Source/YiSan/UI/Public/ULobbyWidget.h`

로비 UI 위젯 클래스입니다.

### 5.1 UI Bindings (meta=(BindWidget))

#### `Btn_Host`

```cpp
UPROPERTY(meta=(BindWidget))
TObjectPtr<UButton> Btn_Host;
```

**Blueprint에서 반드시 생성해야 하는 위젯:** 이름이 정확히 `Btn_Host`여야 함

---

#### `Btn_Join`

```cpp
UPROPERTY(meta=(BindWidget))
TObjectPtr<UButton> Btn_Join;
```

**Blueprint에서 반드시 생성해야 하는 위젯:** 이름이 정확히 `Btn_Join`이어야 함

---

#### `Btn_Disconnect`

```cpp
UPROPERTY(meta=(BindWidget))
TObjectPtr<UButton> Btn_Disconnect;
```

**Blueprint에서 반드시 생성해야 하는 위젯:** 이름이 정확히 `Btn_Disconnect`여야 함

---

#### `TxtBox_IPAddress`

```cpp
UPROPERTY(meta=(BindWidget))
TObjectPtr<UEditableTextBox> TxtBox_IPAddress;
```

**Blueprint에서 반드시 생성해야 하는 위젯:** 이름이 정확히 `TxtBox_IPAddress`여야 함

---

#### `Txt_Status`

```cpp
UPROPERTY(meta=(BindWidget))
TObjectPtr<UTextBlock> Txt_Status;
```

**Blueprint에서 반드시 생성해야 하는 위젯:** 이름이 정확히 `Txt_Status`여야 함

---

### 5.2 Settings

#### `MapName`

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby|Settings")
FString MapName = TEXT("MainMap_WP");
```

**설명:** 호스팅할 맵 이름

**Blueprint에서 수정 가능**

---

#### `MaxPlayers`

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby|Settings")
int32 MaxPlayers = 4;
```

**설명:** 최대 플레이어 수

**Blueprint에서 수정 가능**

---

#### `Port`

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby|Settings")
int32 Port = 7777;
```

**설명:** 서버 포트

**Blueprint에서 수정 가능**

---

### 5.3 Public Functions

#### `UpdateStatusText`

```cpp
UFUNCTION(BlueprintCallable, Category="Lobby|UI")
void UpdateStatusText(const FString& StatusText);
```

**설명:** 상태 텍스트를 업데이트합니다.

**매개변수:**
- `StatusText` (FString): 표시할 상태 메시지

**사용 예시 (Blueprint):**
```
Update Status Text (Status Text: "연결 중...")
```

---

### 5.4 Event Handlers (자동 호출됨)

이 함수들은 UBroadcastManager의 이벤트에 자동으로 바인딩됩니다.

#### `OnSessionHost`

```cpp
UFUNCTION()
void OnSessionHost(const FString& InMapName);
```

**설명:** 세션 호스트 생성 시 호출됩니다.

**동작:**
1. 상태 텍스트 업데이트: "호스트 생성 완료!"
2. Host/Join 버튼 숨김
3. Disconnect 버튼 표시

---

#### `OnSessionJoin`

```cpp
UFUNCTION()
void OnSessionJoin(const FString& Address, int32 InPort);
```

**설명:** 세션 참가 시 호출됩니다.

**동작:**
1. 상태 텍스트 업데이트: "서버 접속 완료!"
2. Host/Join 버튼 숨김
3. Disconnect 버튼 표시

---

#### `OnSessionDisconnect`

```cpp
UFUNCTION()
void OnSessionDisconnect();
```

**설명:** 세션 연결 해제 시 호출됩니다.

**동작:**
1. 상태 텍스트 업데이트: "연결 해제됨"
2. Disconnect 버튼 숨김
3. Host/Join 버튼 표시

---

#### `OnSessionError`

```cpp
UFUNCTION()
void OnSessionError(const FString& ErrorMessage);
```

**설명:** 세션 오류 발생 시 호출됩니다.

**동작:**
1. 상태 텍스트 업데이트: "오류: {ErrorMessage}"

---

## 6. UBroadcastManager

**경로:** `Source/LatteLibrary/Manager/Public/UBroadcastManager.h`

전역 이벤트 버스 역할을 하는 GameInstanceSubsystem입니다.

### 6.1 Multiplayer Events

#### `OnSessionHost`

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionHost, FString, MapName);
UPROPERTY(BlueprintAssignable, Category="Events|Multiplayer")
FOnSessionHost OnSessionHost;
```

**설명:** 세션 호스트 생성 이벤트

**매개변수:**
- `MapName` (FString): 호스팅된 맵 이름

**바인딩 예시 (C++):**
```cpp
if (auto BroadcastManager = UBroadcastManager::Get(GetWorld()))
{
    BroadcastManager->OnSessionHost.AddDynamic(this, &UMyWidget::OnSessionHost);
}
```

**바인딩 예시 (Blueprint):**
```
Event BeginPlay
  → Get Broadcast Manager
  → Bind Event to OnSessionHost
```

---

#### `SendSessionHost`

```cpp
UFUNCTION(BlueprintCallable, Category="Events|Multiplayer")
void SendSessionHost(const FString& MapName);
```

**설명:** 세션 호스트 생성 이벤트를 브로드캐스트합니다.

**호출 위치:** `UYiSanGameInstance::HostGame()`

---

#### `OnSessionJoin`

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionJoin, FString, Address, int32, Port);
UPROPERTY(BlueprintAssignable, Category="Events|Multiplayer")
FOnSessionJoin OnSessionJoin;
```

**설명:** 세션 참가 이벤트

**매개변수:**
- `Address` (FString): 서버 IP 주소
- `Port` (int32): 서버 포트

---

#### `SendSessionJoin`

```cpp
UFUNCTION(BlueprintCallable, Category="Events|Multiplayer")
void SendSessionJoin(const FString& Address, int32 Port);
```

**설명:** 세션 참가 이벤트를 브로드캐스트합니다.

**호출 위치:** `UYiSanGameInstance::JoinGame()`

---

#### `OnSessionDisconnect`

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionDisconnect);
UPROPERTY(BlueprintAssignable, Category="Events|Multiplayer")
FOnSessionDisconnect OnSessionDisconnect;
```

**설명:** 세션 연결 해제 이벤트

---

#### `SendSessionDisconnect`

```cpp
UFUNCTION(BlueprintCallable, Category="Events|Multiplayer")
void SendSessionDisconnect();
```

**설명:** 세션 연결 해제 이벤트를 브로드캐스트합니다.

**호출 위치:** `UYiSanGameInstance::DisconnectFromSession()`

---

#### `OnSessionError`

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionError, FString, ErrorMessage);
UPROPERTY(BlueprintAssignable, Category="Events|Multiplayer")
FOnSessionError OnSessionError;
```

**설명:** 세션 오류 이벤트

**매개변수:**
- `ErrorMessage` (FString): 오류 메시지

---

#### `SendSessionError`

```cpp
UFUNCTION(BlueprintCallable, Category="Events|Multiplayer")
void SendSessionError(const FString& ErrorMessage);
```

**설명:** 세션 오류 이벤트를 브로드캐스트합니다.

**사용 예시:**
```cpp
if (auto BroadcastManager = UBroadcastManager::Get(GetWorld()))
{
    BroadcastManager->SendSessionError(TEXT("서버 연결 실패"));
}
```

---

## 부록: 매크로 및 유틸리티

### DEFINE_SUBSYSTEM_GETTER_INLINE

```cpp
// UBroadcastManager에서 사용
DEFINE_SUBSYSTEM_GETTER_INLINE(UBroadcastManager);
```

**기능:** Subsystem 인스턴스를 쉽게 가져오는 정적 함수 생성

**사용 예시:**
```cpp
// 매크로 덕분에 이렇게 간단하게 사용 가능
if (auto BroadcastManager = UBroadcastManager::Get(GetWorld()))
{
    BroadcastManager->SendMessage(TEXT("Hello"));
}
```

---

### PRINTLOG

```cpp
// GameLogging.h에 정의된 매크로
PRINTLOG(TEXT("[MyClass] Message: %s"), *MyString);
```

**기능:** 포맷팅된 로그 출력

**사용처:** 모든 멀티플레이 함수에서 디버깅용으로 사용

---

## 부록: 네트워크 권한 체크

### HasAuthority()

```cpp
if (!HasAuthority())
    return; // 서버에서만 실행
```

**설명:** 현재 코드가 서버에서 실행 중인지 확인

**사용처:**
- `AYiSanGameMode::StartTour()`
- `AYiSanGameMode::SetTourState()`

---

### GetLocalRole() / GetRemoteRole()

```cpp
ENetRole Role = GetLocalRole();
if (Role == ROLE_Authority)
{
    // 서버 로직
}
else if (Role == ROLE_AutonomousProxy)
{
    // 로컬 플레이어 로직
}
```

**설명:** Actor의 네트워크 역할 확인

**역할:**
- `ROLE_Authority`: 서버
- `ROLE_AutonomousProxy`: 로컬 플레이어 (클라이언트)
- `ROLE_SimulatedProxy`: 원격 플레이어 (다른 클라이언트)

---

**문서 끝**
