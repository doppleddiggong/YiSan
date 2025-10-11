# 언리얼 엔진 통합 작업 계획

## 📋 현재 상태 (음성 서버)

### ✅ 완료된 작업
- **음성 서버**: `http://localhost:4000` (Python FastAPI)
- **STT (Speech-to-Text)**: Whisper medium 모델
- **TTS (Text-to-Speech)**: Google Cloud TTS (ko-KR-Wavenet-D)
- **GPT Agent**: gpt-4o-mini (정조/다산 페르소나)
- **WebSocket**: 실시간 양방향 통신 지원 (`ws://localhost:4000/ws`)
- **환경변수 설정**:
  - `USE_NGROK=false` (로컬 전용)
  - `RELOAD_MODE=false` (안정적 운영)
  - `WHISPER_MODEL_SIZE=medium`
  - `FFMPEG_PATH=E:\MCP\ffmpeg\bin\ffmpeg.exe`

---

## 🎯 언리얼 통합 목표

### 1. HTTP API 통합
**엔드포인트 목록**:
- `POST /stt` - 음성을 텍스트로 변환 (파일 업로드)
- `POST /test/gpt` - GPT Agent에게 질문 (JSON)
- `POST /test/tts` - 텍스트를 음성으로 변환 (JSON → WAV 바이너리)

### 2. WebSocket 실시간 통신
**WebSocket 엔드포인트**: `ws://localhost:4000/ws`

**메시지 흐름**:
```
[사용자 음성 입력]
    ↓
[언리얼 → 서버] WebSocket으로 오디오 전송
    ↓
[서버] STT → GPT Agent → TTS 처리
    ↓
[서버 → 언리얼] WebSocket으로 응답 전송
    ↓
[언리얼] 음성 재생
```

---

## 🔧 언리얼에서 구현할 기능

### Phase 1: HTTP 통신 테스트 ✅
- [x] HTTP Request 노드로 `/test/gpt` 호출 테스트 → `UHttpSystem::RequestTestGPT()`
- [x] JSON 파싱 및 응답 처리 → `FResponseTestGPT`
- [x] `/test/tts` 호출하여 WAV 바이너리 수신 → `UHttpSystem::RequestTestTTS()`

### Phase 2: 마이크 입력 ✅
- [x] 언리얼 마이크 입력 캡처 → `UVoiceRecordSystem`
- [x] 오디오 데이터를 WAV로 인코딩 → `UVoiceFunctionLibrary::ConvertPCM2WAV()`
- [x] `/stt` 엔드포인트로 전송 → `UHttpSystem::RequestSTT()`

### Phase 3: WebSocket 실시간 통신 ✅
- [x] WebSocket 플러그인 활성화
- [x] `ws://localhost:4000/ws` 연결 → `UWebSocketSystem::Connect()`
- [x] JSON 메시지 전송/수신 구현 → `OnMessage_Native()`
- [x] 바이너리 오디오 스트리밍 → `UVoiceListenSystem`

### Phase 4: 통합 대화 시스템 ✅
- [x] 음성 녹음 → STT → GPT → TTS 전체 흐름 통합 → `UVoiceConversationSystem`
- [x] HTTP 방식 및 WebSocket 방식 모두 지원
- [x] 이벤트 델리게이트를 통한 상태 알림

### Phase 5: UI/UX (구현 대기)
- [ ] 녹음 버튼 (누르고 말하기)
- [ ] 대화 로그 표시 (사용자 질문 + GPT 응답)
- [ ] 음성 재생 상태 표시
- [ ] 연결 상태 표시 (서버 연결 여부)

---

## 📡 API 명세

### 1. POST /stt (음성 → 텍스트)
**요청**:
```http
POST http://localhost:4000/stt
Content-Type: multipart/form-data

file: [audio.wav or audio.webm]
```

**응답**:
```json
{
  "text": "전하, 신이 말씀드리고자 하는 바는..."
}
```

---

### 2. POST /test/gpt (GPT 질문)
**요청**:
```http
POST http://localhost:4000/test/gpt
Content-Type: application/json

{
  "text": "정조 시대의 정치 개혁에 대해 알려주세요"
}
```

**응답**:
```json
{
  "response": "신(臣), 전하! 정조 시대의 정치 개혁은..."
}
```

---

### 3. POST /test/tts (텍스트 → 음성)
**요청**:
```http
POST http://localhost:4000/test/tts
Content-Type: application/json

{
  "text": "안녕하세요",
  "speaking_rate": 0.88,  // 선택사항 (0.25~4.0)
  "pitch": -3.0            // 선택사항 (-20.0~20.0)
}
```

**응답**:
- Content-Type: `audio/wav`
- 바이너리 WAV 데이터 (24kHz, 16-bit PCM)

---

### 4. WebSocket /ws (실시간 통신)
**연결**: `ws://localhost:4000/ws`

**서버 → 클라이언트 메시지 예시**:
```json
{
  "type": "transcription",
  "text": "전하께 여쭙고자 하는 바가 있사옵니다",
  "status": "success"
}
```

```json
{
  "type": "agent_response",
  "text": "신(臣), 말씀하시오.",
  "status": "success"
}
```

```json
{
  "type": "audio",
  "audio_base64": "UklGRiQAAABXQVZFZm10...",
  "format": "wav",
  "status": "success"
}
```

```json
{
  "type": "error",
  "message": "TTS generation failed",
  "status": "error"
}
```

---

## 🛠️ 언리얼 구현 체크리스트

### 필요한 플러그인
- [ ] **HTTP 통신**: 언리얼 내장 `HTTP` 모듈 (기본 제공)
- [ ] **WebSocket**: `WebSocketNetworking` 플러그인 또는 써드파티 플러그인
- [ ] **JSON 파싱**: `Json`, `JsonUtilities` 모듈 (기본 제공)
- [ ] **음성 캡처**: `VoiceCapture` 모듈 (기본 제공)
- [ ] **오디오 재생**: `AudioMixer` 또는 `Sound` 모듈 (기본 제공)

### 블루프린트 구조 (권장)
```
BP_VoiceManager (Actor Component)
├── MicrophoneCapture (Voice Capture Component)
├── HTTPRequestHandler (Custom Component)
├── WebSocketClient (Custom Component)
├── AudioPlayer (Audio Component)
└── UI_ChatLog (Widget)
```

---

## 🚀 시작하기

### 1. 음성 서버 실행
```bash
cd E:\UE\Dasan
python run.py
```
- 서버 주소: `http://localhost:4000`
- Swagger UI: `http://localhost:4000/docs`

### 2. 언리얼에서 테스트
1. HTTP Request로 `/test/gpt` 호출
2. 응답 JSON 파싱
3. 결과를 화면에 출력

### 3. 점진적 확장
1. TTS 연동 → 음성 재생
2. 마이크 입력 → STT 연동
3. WebSocket 실시간 통신
4. UI/UX 개선

---

## 📌 참고 사항

### 서버 재시작 방법
```bash
# 서버 종료: Ctrl+C
# 또는 강제 종료
taskkill /F /IM python.exe

# 서버 시작
python run.py
```

### 환경변수 변경 (필요시)
`.env` 파일 수정 후 서버 재시작:
- `USE_NGROK=true`: 외부 접속 활성화
- `WHISPER_MODEL_SIZE=small`: 모델 크기 변경
- `RELOAD_MODE=true`: 개발 모드 (자동 재시작)

### 디버깅
- 서버 로그: 터미널 출력 확인
- API 테스트: Swagger UI (`http://localhost:4000/docs`)
- WebSocket 테스트: 브라우저 개발자 도구 또는 Postman

---

## 🎓 다음 단계

1. **언리얼 프로젝트 열기**
2. **HTTP Request 블루프린트 노드 테스트**
3. **JSON 파싱 및 로그 출력**
4. **TTS 음성 재생 구현**
5. **마이크 입력 캡처**
6. **WebSocket 연결**

---

## 📞 API 엔드포인트 요약

| Method | Endpoint | 설명 | 요청 | 응답 |
|--------|----------|------|------|------|
| GET | `/health` | 서버 상태 확인 | - | `{"status": "ok"}` |
| POST | `/stt` | 음성 → 텍스트 | 오디오 파일 | `{"text": "..."}` |
| POST | `/test/gpt` | GPT 질문 | `{"text": "..."}` | `{"response": "..."}` |
| POST | `/test/tts` | 텍스트 → 음성 | `{"text": "..."}` | WAV 바이너리 |
| WS | `/ws` | 실시간 통신 | JSON/Binary | JSON/Binary |

---

## 💡 구현된 시스템 사용 가이드

### 1️⃣ HTTP 방식 음성 대화 (간단한 방식)

```cpp
// 블루프린트 또는 C++에서 사용
UVoiceConversationSystem* VoiceSystem = UVoiceConversationSystem::Get(GetWorld());

// 이벤트 바인딩
VoiceSystem->OnTranscriptionReceived.AddDynamic(this, &AMyActor::OnSTTReceived);
VoiceSystem->OnGPTResponseReceived.AddDynamic(this, &AMyActor::OnGPTReceived);
VoiceSystem->OnCompleted.AddDynamic(this, &AMyActor::OnConversationCompleted);

// 녹음 시작
VoiceSystem->StartRecording();

// 사용자가 말함...

// 녹음 중지 → 자동으로 STT → GPT → TTS 처리
VoiceSystem->StopRecording();
```

### 2️⃣ WebSocket 방식 실시간 음성 대화

```cpp
UVoiceConversationSystem* VoiceSystem = UVoiceConversationSystem::Get(GetWorld());

// WebSocket 연결
VoiceSystem->ConnectWebSocket();

// 오디오 데이터 전송 (실시간 스트리밍)
TArray<uint8> AudioChunk = GetMicrophoneAudioChunk();
VoiceSystem->SendAudioToWebSocket(AudioChunk);

// TTS 요청
VoiceSystem->RequestTTSViaWebSocket(TEXT("안녕하세요"));

// 연결 해제
VoiceSystem->DisconnectWebSocket();
```

### 3️⃣ 직접 텍스트 질문 (음성 녹음 없이)

```cpp
UVoiceConversationSystem* VoiceSystem = UVoiceConversationSystem::Get(GetWorld());

// GPT에게 바로 질문
VoiceSystem->AskGPTDirectly(TEXT("정조 시대의 정치 개혁에 대해 알려주세요"));
```

---

## 🏗️ 구현된 시스템 아키텍처

```
┌─────────────────────────────────────────────────────────────────┐
│                    UVoiceConversationSystem                     │
│                      (통합 대화 관리자)                           │
└─────────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              ▼               ▼               ▼
    ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
    │ UHttpSystem │  │UWebSocketSys│  │UVoiceRecord │
    │             │  │    tem      │  │   System    │
    │ • STT       │  │ • 실시간    │  │ • 마이크    │
    │ • GPT       │  │   통신      │  │   입력      │
    │ • TTS       │  │ • 스트리밍  │  │ • WAV 저장  │
    └─────────────┘  └─────────────┘  └─────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │UVoiceListenSys  │
                    │                 │
                    │ • 오디오 재생   │
                    │ • 스트리밍      │
                    └─────────────────┘
```

### 주요 컴포넌트

1. **UVoiceConversationSystem** (GameInstanceSubsystem)
   - 전체 음성 대화 흐름 관리
   - HTTP 및 WebSocket 방식 모두 지원
   - 이벤트 기반 상태 알림

2. **UHttpSystem** (GameInstanceSubsystem)
   - REST API 요청 처리
   - STT, GPT, TTS 엔드포인트 지원
   - 멀티파트 파일 업로드 지원

3. **UWebSocketSystem** (GameInstanceSubsystem)
   - 실시간 양방향 통신
   - 오디오 스트리밍
   - JSON 메시지 처리

4. **UVoiceRecordSystem** (ActorComponent)
   - 마이크 입력 캡처
   - PCM → WAV 변환
   - 파일 저장

5. **UVoiceListenSystem** (ActorComponent)
   - WebSocket 오디오 스트림 재생
   - USoundWaveProcedural 사용
   - 실시간 오디오 큐잉

---

## 🎯 네트워크 설정

`Config/DefaultMyNetwork.ini` 파일에서 서버 주소 설정:

```ini
[/Script/YiSan.CustomNetworkSettings]
DefaultMode=Dev
DevConfig=(BaseUrl="http://127.0.0.1",Port=4000,WebSocketUrl="ws://127.0.0.1:4000/ws")
StageConfig=(BaseUrl="http://100.100.53.32",Port=9999,WebSocketUrl="ws://100.100.53.32:9999/ws")
LiveConfig=(BaseUrl="http://172.16.20.104",Port=8000,WebSocketUrl="ws://172.16.20.104:8000/ws")
```

현재 **Dev 모드**로 설정되어 있으며, 로컬 음성 서버(`localhost:4000`)와 통신합니다.

---

## 🔥 빠른 시작 (Quick Start)

### 1. 음성 서버 실행
```bash
cd E:\UE\Dasan
python run.py
```

### 2. 언리얼 에디터에서 테스트

#### 블루프린트 예제
1. **BeginPlay**에서:
   - `Get Voice Conversation System` 노드 추가
   - `Bind Event to On Transcription Received` 추가
   - `Bind Event to On GPT Response Received` 추가

2. **키 입력 (예: T키)**:
   - `Start Recording` 호출

3. **키 릴리즈 (T키 떼기)**:
   - `Stop Recording` 호출 → 자동으로 STT → GPT → TTS 처리

#### C++ 예제
```cpp
void AMyPlayerController::SetupVoiceConversation()
{
    UVoiceConversationSystem* VoiceSystem = UVoiceConversationSystem::Get(GetWorld());

    VoiceSystem->OnTranscriptionReceived.AddDynamic(this, &AMyPlayerController::HandleSTT);
    VoiceSystem->OnGPTResponseReceived.AddDynamic(this, &AMyPlayerController::HandleGPT);
    VoiceSystem->OnTTSStarted.AddDynamic(this, &AMyPlayerController::HandleTTS);
    VoiceSystem->OnCompleted.AddDynamic(this, &AMyPlayerController::HandleCompleted);
}

void AMyPlayerController::OnPressVoiceButton()
{
    UVoiceConversationSystem::Get(GetWorld())->StartRecording();
}

void AMyPlayerController::OnReleaseVoiceButton()
{
    UVoiceConversationSystem::Get(GetWorld())->StopRecording();
}
```

---

**준비 완료!** 언리얼 프로젝트에서 Claude CLI를 열고 이 파일을 참고하여 작업을 시작하세요! 🚀
