# YiSan 멀티플레이 구현 가이드

**작성일:** 2025-10-20
**버전:** 1.0
**대상:** Blueprint 개발자, 레벨 디자이너

---

## 📑 목차

1. [시작하기 전에](#1-시작하기-전에)
2. [C++ 코드 컴파일](#2-c-코드-컴파일)
3. [LobbyMap 생성](#3-lobbymap-생성)
4. [WBP_LobbyWidget 생성](#4-wbp_lobbywidget-생성)
5. [MainMap_WP 설정](#5-mainmap_wp-설정)
6. [프로젝트 설정](#6-프로젝트-설정)
7. [테스트 가이드](#7-테스트-가이드)
8. [트러블슈팅](#8-트러블슈팅)

---

## 1. 시작하기 전에

### 1.1 필수 요구사항

- ✅ Unreal Engine 5.3 이상
- ✅ Visual Studio 2022 (C++ 개발 도구)
- ✅ YiSan 프로젝트 소스 코드
- ✅ Git (버전 관리)

### 1.2 필요한 파일 확인

다음 C++ 파일들이 모두 존재하는지 확인하세요:

```
YiSan/Source/
├─ YiSan/
│  ├─ Loading/
│  │  ├─ Public/YiSanGameInstance.h         ✅
│  │  └─ Private/YiSanGameInstance.cpp      ✅
│  ├─ Environment/
│  │  ├─ Public/ALobbyGameMode.h            ✅
│  │  ├─ Private/ALobbyGameMode.cpp         ✅
│  │  ├─ Public/AYiSanGameMode.h            ✅
│  │  └─ Private/AYiSanGameMode.cpp         ✅
│  └─ UI/
│     ├─ Public/ULobbyWidget.h              ✅
│     └─ Private/ULobbyWidget.cpp           ✅
└─ LatteLibrary/
   ├─ Character/
   │  ├─ Public/APlayerControl.h            ✅
   │  └─ Private/APlayerControl.cpp         ✅
   └─ Manager/
      ├─ Public/UBroadcastManager.h         ✅
      └─ Private/UBroadcastManager.cpp      ✅
```

---

## 2. C++ 코드 컴파일

### 2.1 언리얼 에디터 닫기

- 현재 언리얼 에디터가 열려 있으면 **반드시 닫아주세요**.

### 2.2 프로젝트 재생성

1. `YiSan.uproject` 파일 우클릭
2. **"Generate Visual Studio project files"** 선택
3. 완료될 때까지 대기

### 2.3 Visual Studio에서 컴파일

1. `YiSan.sln` 파일 열기 (Visual Studio 2022)
2. 솔루션 구성: **Development Editor**
3. 플랫폼: **Win64**
4. 메뉴: **빌드 → 솔루션 빌드** (Ctrl+Shift+B)
5. 빌드 성공 확인:
   ```
   ========== 빌드: 성공 2, 실패 0, 최신 0, 건너뛴 0 ==========
   ```

### 2.4 언리얼 에디터 실행

- `YiSan.uproject` 파일 더블클릭
- 에디터 로딩 완료 대기

---

## 3. LobbyMap 생성

### 3.1 새 레벨 생성

1. **파일 → 새 레벨** (Ctrl+N)
2. **빈 레벨** 선택
3. **다른 이름으로 저장**: `Content/YiSan/Maps/LobbyMap.umap`

### 3.2 기본 액터 추가

#### 3.2.1 Player Start 추가

1. **Place Actors** 패널에서 **Player Start** 검색
2. 뷰포트에 드래그 앤 드롭
3. 위치: `X=0, Y=0, Z=100`

#### 3.2.2 Light 추가

1. **Directional Light** 추가
2. 위치: `X=0, Y=0, Z=300`
3. 회전: `Pitch=-45, Yaw=0, Roll=0`

#### 3.2.3 Sky Atmosphere 추가 (선택 사항)

1. **Sky Atmosphere** 추가
2. 기본 설정 유지

#### 3.2.4 Post Process Volume 추가 (선택 사항)

1. **Post Process Volume** 추가
2. **Infinite Extent (Unbound)** 체크

### 3.3 World Settings 설정

1. **Window → World Settings** (Shift+K)
2. **World Settings** 패널에서 다음 설정:

```
┌─────────────────────────────────────┐
│ World Settings                       │
├─────────────────────────────────────┤
│ Game Mode                            │
│   Game Mode Override: ALobbyGameMode │ ← 여기 설정!
└─────────────────────────────────────┘
```

### 3.4 GameMode 선택 방법

1. **Game Mode Override** 드롭다운 클릭
2. 검색: `Lobby`
3. **ALobbyGameMode** 선택
4. 선택 후 다음 속성 확인:
   ```
   Default Pawn Class: BP_PlayerActor (기본값)
   Player Controller Class: APlayerControl
   ```

### 3.5 레벨 저장

- **파일 → 저장** (Ctrl+S)

---

## 4. WBP_LobbyWidget 생성

### 4.1 Widget Blueprint 생성

1. **Content Browser**에서 `Content/YiSan/UI/` 폴더로 이동
   - 폴더가 없으면 생성: **우클릭 → New Folder → "UI"**
2. **우클릭 → User Interface → Widget Blueprint**
3. **Pick Parent Class** 창에서:
   - **All Classes** 체크
   - 검색: `LobbyWidget`
   - **ULobbyWidget** 선택 ⚠️ (기본 UserWidget이 아님!)
4. 이름: `WBP_LobbyWidget`

### 4.2 위젯 구조 설계

Designer 모드에서 다음 위젯 구조를 만듭니다:

```
Canvas Panel (Root)
└─ Vertical Box
    ├─ Text Block (제목)
    ├─ Spacer (20px)
    ├─ Text Block (Txt_Status) ← ⚠️ 이름 정확히!
    ├─ Spacer (20px)
    ├─ Editable Text Box (TxtBox_IPAddress) ← ⚠️ 이름 정확히!
    ├─ Spacer (10px)
    ├─ Button (Btn_Host) ← ⚠️ 이름 정확히!
    │   └─ Text Block (텍스트: "Host Game")
    ├─ Spacer (10px)
    ├─ Button (Btn_Join) ← ⚠️ 이름 정확히!
    │   └─ Text Block (텍스트: "Join Game")
    ├─ Spacer (10px)
    └─ Button (Btn_Disconnect) ← ⚠️ 이름 정확히!
        └─ Text Block (텍스트: "Disconnect")
```

### 4.3 단계별 위젯 생성

#### 4.3.1 Canvas Panel (Root)

- 이미 존재함 (기본값)

#### 4.3.2 Vertical Box 추가

1. **Palette**에서 **Vertical Box** 검색
2. **Canvas Panel** 위로 드래그
3. **Anchors** 설정:
   - Presets: **Center** 선택
4. **Position**:
   - Position X: `0`
   - Position Y: `0`
5. **Size To Content** 체크

#### 4.3.3 제목 Text Block 추가

1. **Palette**에서 **Text Block** 검색
2. **Vertical Box** 위로 드래그
3. **Details** 패널:
   ```
   Content → Text: "YiSan Multiplayer Lobby"
   Appearance → Font:
     - Size: 48
     - Typeface: Bold
   ```

#### 4.3.4 Spacer 추가

1. **Palette**에서 **Spacer** 검색
2. **Vertical Box** 위로 드래그
3. **Details** 패널:
   ```
   Size → Y: 20
   ```

#### 4.3.5 Txt_Status (상태 표시)

1. **Text Block** 추가
2. **⚠️ 중요: 이름을 정확히 `Txt_Status`로 변경!**
   - Hierarchy에서 우클릭 → Rename → `Txt_Status`
3. **Details** 패널:
   ```
   Content → Text: "대기 중..."
   Appearance → Font:
     - Size: 24
     - Color: Yellow (R=1, G=1, B=0)
   ```

#### 4.3.6 Spacer 추가 (20px)

- 위와 동일

#### 4.3.7 TxtBox_IPAddress (IP 입력)

1. **Editable Text Box** 추가
2. **⚠️ 중요: 이름을 정확히 `TxtBox_IPAddress`로 변경!**
3. **Details** 패널:
   ```
   Content → Hint Text: "Enter IP Address (e.g., 192.168.0.10)"
   Appearance → Font:
     - Size: 20
   ```
4. **Size**:
   - Width Override: `400`
   - Height Override: `40`

#### 4.3.8 Spacer 추가 (10px)

```
Size → Y: 10
```

#### 4.3.9 Btn_Host (Host 버튼)

1. **Button** 추가
2. **⚠️ 중요: 이름을 정확히 `Btn_Host`로 변경!**
3. **Button** 안에 **Text Block** 추가:
   ```
   Text: "🎮 Host Game"
   Font Size: 24
   ```
4. **Button** 설정:
   ```
   Style → Normal:
     - Tint: Green (R=0, G=0.5, B=0)
   Size:
     - Width Override: 400
     - Height Override: 50
   ```

#### 4.3.10 Spacer 추가 (10px)

- 위와 동일

#### 4.3.11 Btn_Join (Join 버튼)

1. **Button** 추가
2. **⚠️ 중요: 이름을 정확히 `Btn_Join`으로 변경!**
3. **Button** 안에 **Text Block** 추가:
   ```
   Text: "🔗 Join Game"
   Font Size: 24
   ```
4. **Button** 설정:
   ```
   Style → Normal:
     - Tint: Blue (R=0, G=0.3, B=0.8)
   Size:
     - Width Override: 400
     - Height Override: 50
   ```

#### 4.3.12 Spacer 추가 (10px)

- 위와 동일

#### 4.3.13 Btn_Disconnect (Disconnect 버튼)

1. **Button** 추가
2. **⚠️ 중요: 이름을 정확히 `Btn_Disconnect`로 변경!**
3. **Button** 안에 **Text Block** 추가:
   ```
   Text: "❌ Disconnect"
   Font Size: 24
   ```
4. **Button** 설정:
   ```
   Style → Normal:
     - Tint: Red (R=0.8, G=0, B=0)
   Size:
     - Width Override: 400
     - Height Override: 50
   ```

### 4.4 위젯 이름 검증

**⚠️ 매우 중요: 다음 이름들이 정확히 일치하는지 확인하세요!**

```
Hierarchy에서 확인:
✅ Txt_Status
✅ TxtBox_IPAddress
✅ Btn_Host
✅ Btn_Join
✅ Btn_Disconnect
```

### 4.5 Properties 설정 (선택 사항)

1. **Graph** 모드로 전환
2. **Class Defaults** 버튼 클릭 (우측 상단)
3. **Details** 패널에서:
   ```
   Lobby | Settings:
     - Map Name: "MainMap_WP"
     - Max Players: 4
     - Port: 7777
   ```

### 4.6 컴파일 및 저장

1. **컴파일** 버튼 클릭 (좌측 상단)
2. 에러가 없는지 확인
   - ❌ 에러 예시: `"Btn_Host" 위젯을 찾을 수 없음`
   - → 위젯 이름을 정확히 맞춰야 함!
3. **저장** 버튼 클릭

---

## 5. LobbyMap에 위젯 추가

### 5.1 Level Blueprint 열기

1. **LobbyMap** 열기
2. **Blueprints → Open Level Blueprint** (Ctrl+K)

### 5.2 BeginPlay 이벤트 구성

Level Blueprint에 다음 로직을 구성하세요:

```
┌─────────────────────────────────────────────────────────────┐
│ Event BeginPlay                                              │
└────┬────────────────────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────────────────────────┐
│ Create Widget (Class: WBP_LobbyWidget)                       │
│   Owning Player: [Get Player Controller] → Return Value     │
└────┬────────────────────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────────────────────────┐
│ Add to Viewport                                              │
│   Target: [위 Create Widget의 Return Value]                 │
└────┬────────────────────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────────────────────────┐
│ Get Player Controller                                        │
└────┬────────────────────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────────────────────────┐
│ Set Input Mode UI Only                                       │
│   Player Controller: [위 Get Player Controller]             │
│   Widget to Focus: [Create Widget의 Return Value]           │
└────┬────────────────────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────────────────────────┐
│ Set Show Mouse Cursor                                        │
│   Target: [Get Player Controller]                           │
│   bShow Mouse Cursor: TRUE (체크)                           │
└─────────────────────────────────────────────────────────────┘
```

### 5.3 상세 연결 방법

1. **Event BeginPlay** 노드 찾기 (이미 존재)

2. **Create Widget** 노드 추가:
   - 우클릭 → 검색: `Create Widget`
   - **Class**: `WBP_LobbyWidget` 선택
   - **Owning Player**:
     - 우클릭 → 검색: `Get Player Controller`
     - Player Index: `0`
     - 연결: `Get Player Controller` → `Owning Player`

3. **Add to Viewport** 노드 추가:
   - 우클릭 → 검색: `Add to Viewport`
   - 연결: `Create Widget의 Return Value` → `Add to Viewport의 Target`

4. **Set Input Mode UI Only** 노드 추가:
   - 우클릭 → 검색: `Set Input Mode UI Only`
   - **Player Controller**: 위의 `Get Player Controller` 재사용
   - **Widget to Focus**: `Create Widget의 Return Value` 연결

5. **Set Show Mouse Cursor** 노드 추가:
   - 우클릭 → 검색: `Set Show Mouse Cursor`
   - **Target**: `Get Player Controller` 연결
   - **bShow Mouse Cursor**: ✅ 체크

### 5.4 컴파일 및 저장

- **컴파일** 버튼 클릭
- **저장** (Ctrl+S)

---

## 6. MainMap_WP 설정

### 6.1 MainMap_WP 열기

- **Content Browser**에서 `MainMap_WP.umap` 열기

### 6.2 World Settings 설정

1. **Window → World Settings** (Shift+K)
2. **Game Mode Override**: `BP_YiSanGameMode` 선택
   - 주의: `AYiSanGameMode`를 상속받은 Blueprint 클래스여야 함
   - 없으면 새로 생성:
     1. **Content Browser**에서 우클릭
     2. **Blueprint Class** 선택
     3. **All Classes** → 검색: `YiSanGameMode`
     4. **AYiSanGameMode** 선택
     5. 이름: `BP_YiSanGameMode`

### 6.3 GameMode 설정 확인

```
┌─────────────────────────────────────┐
│ World Settings                       │
├─────────────────────────────────────┤
│ Game Mode                            │
│   Game Mode Override:                │
│     BP_YiSanGameMode                 │ ← 여기 설정!
│   Default Pawn Class:                │
│     BP_PlayerActor                   │
│   Player Controller Class:           │
│     APlayerControl                   │
└─────────────────────────────────────┘
```

### 6.4 저장

- **파일 → 저장** (Ctrl+S)

---

## 7. 프로젝트 설정

### 7.1 DefaultEngine.ini 설정

1. 프로젝트 폴더의 `Config/DefaultEngine.ini` 파일 열기
2. 다음 설정 추가 (이미 있으면 수정):

```ini
[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemUtils.IpNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")

[/Script/OnlineSubsystemUtils.IpNetDriver]
NetServerMaxTickRate=120
MaxNetTickRate=120
MaxInternetClientRate=100000
MaxClientRate=100000

[/Script/Engine.Player]
ConfiguredInternetSpeed=100000
ConfiguredLanSpeed=200000

[SystemSettings]
net.MaxRepArraySize=2048
net.MaxRepArrayMemory=16384
```

### 7.2 프로젝트 설정 확인

1. **Edit → Project Settings**
2. **Maps & Modes** 섹션:
   ```
   Default Maps:
     - Editor Startup Map: LobbyMap
     - Game Default Map: LobbyMap

   Default Modes:
     - Default GameMode: ALobbyGameMode
   ```

### 7.3 저장 및 재시작

- **Project Settings** 창 닫기
- 에디터 재시작 (선택 사항, 권장)

---

## 8. 테스트 가이드

### 8.1 단일 플레이어 테스트

#### 8.1.1 Host 테스트

1. **LobbyMap** 열기
2. **Play** 버튼 클릭 (Alt+P)
3. 로비 UI 확인:
   - ✅ Host 버튼 보임
   - ✅ Join 버튼 보임
   - ✅ Disconnect 버튼 숨김
   - ✅ 상태: "대기 중..."
4. **Host 버튼 클릭**
5. 예상 동작:
   - MainMap_WP로 전환
   - 플레이어 생성됨
   - DasanNPC가 보임
6. **성공!** ✅

#### 8.1.2 Join 테스트 (혼자 테스트)

1. **LobbyMap** 열기
2. **Play** 버튼 클릭
3. IP 입력: `127.0.0.1` (로컬호스트)
4. **Join 버튼 클릭**
5. 예상 동작:
   - 서버가 없으면 연결 실패 메시지
   - 정상 (Host를 먼저 실행해야 함)

---

### 8.2 멀티플레이어 테스트 (2인 플레이)

#### 8.2.1 Editor Preferences 설정

1. **Edit → Editor Preferences**
2. 검색: `Multiplayer`
3. **Play in Editor** 섹션:
   ```
   Multiplayer Options:
     - Number of Players: 2
     - Net Mode: Play As Listen Server
   ```
4. **창 크기 조정** (선택 사항):
   ```
   New Editor Window Size:
     - Width: 800
     - Height: 600
   ```

#### 8.2.2 2인 플레이 실행

1. **LobbyMap** 열기
2. **Play** 버튼 옆 드롭다운 클릭
3. **Number of Players: 2** 확인
4. **Play** 버튼 클릭

5. **결과:**
   - 창 2개가 열림:
     - 왼쪽: Server (Player 1)
     - 오른쪽: Client (Player 2)

#### 8.2.3 Host 동작 (Player 1)

**Player 1 창 (왼쪽):**
1. **Host 버튼 클릭**
2. MainMap_WP로 전환
3. 플레이어 생성 확인

#### 8.2.4 Join 동작 (Player 2)

**Player 2 창 (오른쪽):**
1. IP 입력: `127.0.0.1`
2. **Join 버튼 클릭**
3. MainMap_WP로 전환
4. Player 1과 같은 맵에 합류

#### 8.2.5 검증

**양쪽 창에서 확인:**
- ✅ Player 1이 Player 2를 볼 수 있음
- ✅ Player 2가 Player 1을 볼 수 있음
- ✅ DasanNPC가 양쪽에 보임
- ✅ 움직임이 동기화됨

**성공!** 🎉

---

### 8.3 난입(Join In Progress) 테스트

#### 8.3.1 시나리오

1. Player 1이 Host로 게임 시작
2. Player 1이 게임 플레이 중 (투어 진행)
3. Player 2가 나중에 Join
4. Player 2가 현재 게임 상태로 합류

#### 8.3.2 실행 방법

**Player 1 (Host):**
1. LobbyMap 시작
2. Host 버튼 클릭
3. MainMap_WP로 이동
4. 게임 플레이 시작 (예: DasanNPC와 상호작용)

**Player 2 (Client) - 1분 후:**
1. 새 PIE 세션 시작 (Play 버튼 한 번 더 클릭)
2. LobbyMap에서 IP 입력: `127.0.0.1`
3. Join 버튼 클릭

**Player 2 검증:**
- ✅ MainMap_WP로 즉시 이동
- ✅ Player 1이 이미 있는 위치에 합류
- ✅ DasanNPC의 현재 상태가 보임
- ✅ 투어가 진행 중이면 그 상태가 동기화됨

**성공!** 난입 허용이 작동합니다! ✅

---

### 8.4 실제 네트워크 테스트 (같은 LAN)

#### 8.4.1 호스트 PC 설정

1. **Windows 방화벽 설정:**
   - 제어판 → Windows Defender 방화벽
   - **고급 설정** 클릭
   - **인바운드 규칙** → **새 규칙**
   - **포트** 선택 → **TCP 및 UDP**
   - **특정 로컬 포트**: `7777`
   - **연결 허용** 선택
   - 이름: `UE5 YiSan Multiplayer`

2. **IP 주소 확인:**
   - 명령 프롬프트 (cmd) 열기
   - `ipconfig` 입력
   - **IPv4 주소** 확인 (예: `192.168.0.10`)

3. **언리얼 에디터 실행:**
   - LobbyMap 열기
   - Host 버튼 클릭
   - MainMap_WP로 이동
   - 대기...

#### 8.4.2 클라이언트 PC 설정

1. **언리얼 에디터 실행** (같은 프로젝트)
2. **LobbyMap** 열기
3. **IP 입력**: `192.168.0.10` (호스트 PC의 IP)
4. **Join 버튼 클릭**
5. MainMap_WP로 이동
6. 호스트와 합류 성공!

**성공!** 실제 네트워크 멀티플레이가 작동합니다! 🎉

---

### 8.5 Tailscale MagicDNS 테스트 (선택 사항)

#### 8.5.1 Tailscale 설치

1. [Tailscale 다운로드](https://tailscale.com/download)
2. 설치 및 로그인
3. 양쪽 PC에 모두 설치

#### 8.5.2 호스트 PC

1. Tailscale 앱 열기
2. 호스트 이름 확인 (예: `dopple-pc`)
3. 언리얼 에디터에서 Host 버튼 클릭

#### 8.5.3 클라이언트 PC

1. IP 입력란에 **호스트 이름** 입력: `dopple-pc`
2. Join 버튼 클릭
3. **자동으로 IP 해석됨!**

**장점:**
- ✅ IP 주소 외울 필요 없음
- ✅ 공인 IP 불필요
- ✅ 방화벽 설정 간소화

---

## 9. 트러블슈팅

### 9.1 컴파일 오류

#### 오류: `Cannot find file 'YiSanGameInstance.h'`

**원인:** 헤더 파일 경로 문제

**해결:**
1. Visual Studio에서 `#include` 경로 확인
2. 상대 경로 사용: `#include "YiSan/Loading/Public/YiSanGameInstance.h"`

---

#### 오류: `Unresolved external symbol`

**원인:** 링커 오류, 함수 구현 누락

**해결:**
1. `.cpp` 파일에 함수 구현이 있는지 확인
2. 클린 빌드:
   - Visual Studio: **빌드 → 솔루션 정리**
   - 다시 빌드: **빌드 → 솔루션 빌드**

---

### 9.2 위젯 오류

#### 오류: `Widget 'Btn_Host' was not found`

**원인:** 위젯 이름 불일치

**해결:**
1. WBP_LobbyWidget 열기
2. Hierarchy에서 위젯 이름 확인:
   - 정확히 `Btn_Host`, `Btn_Join`, `Btn_Disconnect`
   - `TxtBox_IPAddress`, `Txt_Status`
3. 대소문자 정확히 맞춰야 함!

---

### 9.3 네트워크 연결 오류

#### 오류: `Travel failed - Unable to connect`

**원인:** 서버 IP 또는 포트 문제

**해결:**
1. **Host가 실행 중인지 확인**
2. **IP 주소 확인:**
   - `ipconfig` 명령으로 IPv4 주소 확인
3. **방화벽 체크:**
   - 포트 7777이 열려 있는지 확인
4. **동일 네트워크 확인:**
   - 같은 Wi-Fi 또는 LAN에 연결되어 있는지

---

#### 오류: `No Pawn after PostLogin`

**원인:** GameMode의 Pawn 생성 실패

**해결:**
1. World Settings 확인:
   - **Default Pawn Class**가 설정되어 있는지
2. AYiSanGameMode의 `PostLogin()` 구현 확인:
   - `SpawnDefaultPawnFor()` 호출되는지

---

### 9.4 복제(Replication) 문제

#### 문제: `난입 플레이어가 DasanNPC를 볼 수 없음`

**원인:** GameState 복제 설정 누락

**해결:**
1. `AYisanGameState.h` 확인:
   ```cpp
   UPROPERTY(Replicated, BlueprintReadOnly, Category="Tour")
   TObjectPtr<ADasanActor> DasanNPC;
   ```
2. `GetLifetimeReplicatedProps()` 구현 확인:
   ```cpp
   DOREPLIFETIME(AYisanGameState, DasanNPC);
   ```

---

### 9.5 UI 문제

#### 문제: `로비 위젯이 표시되지 않음`

**원인:** Level Blueprint 설정 누락

**해결:**
1. LobbyMap 열기
2. Level Blueprint (Ctrl+K) 열기
3. `Event BeginPlay` → `Create Widget` → `Add to Viewport` 확인

---

#### 문제: `버튼 클릭이 작동하지 않음`

**원인:** Input Mode 설정 문제

**해결:**
1. Level Blueprint에서 `Set Input Mode UI Only` 노드 확인
2. `Set Show Mouse Cursor` 노드 확인 (TRUE로 설정)

---

## 10. 추가 기능 아이디어

### 10.1 플레이어 목록 표시

- GameState에 플레이어 배열 저장
- UI에 리스트로 표시

### 10.2 채팅 시스템

- `UMainWidget`의 기존 채팅 기능 활용
- RPC를 통한 메시지 브로드캐스트

### 10.3 투어 동기화

- Host만 투어 시작 가능
- 모든 플레이어가 같은 설명 듣기

### 10.4 보이스 채팅

- 기존 `UVoiceConversationSystem` 확장
- WebSocket을 통한 음성 스트리밍

---

## 부록: 체크리스트

### ✅ C++ 코드 컴파일 완료

- [ ] Visual Studio 빌드 성공
- [ ] 언리얼 에디터 정상 실행

### ✅ LobbyMap 설정 완료

- [ ] 레벨 생성됨
- [ ] GameMode: ALobbyGameMode
- [ ] Player Start 배치됨

### ✅ WBP_LobbyWidget 설정 완료

- [ ] Parent Class: ULobbyWidget
- [ ] 위젯 이름 정확히 설정:
  - [ ] Btn_Host
  - [ ] Btn_Join
  - [ ] Btn_Disconnect
  - [ ] TxtBox_IPAddress
  - [ ] Txt_Status
- [ ] Level Blueprint에 추가됨

### ✅ MainMap_WP 설정 완료

- [ ] GameMode: BP_YiSanGameMode

### ✅ 테스트 완료

- [ ] 단일 플레이어 Host 테스트
- [ ] 2인 플레이 테스트
- [ ] 난입 테스트
- [ ] (선택) 실제 네트워크 테스트

---

**구현 가이드 끝**

이제 YiSan 프로젝트에서 멀티플레이를 즐기실 수 있습니다! 🎉
