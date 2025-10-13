# Voice Server API Documentation

이 문서는 Voice Server에서 제공하는 HTTP 및 WebSocket API에 대한 명세서입니다. Unreal Engine 클라이언트 개발 시 이 문서를 참고하여 API를 검증하고 연동할 수 있습니다.

**기본 URL**: `http://<서버_주소>:<포트>` (예: `http://127.0.0.1:8000`)

---

## 1. HTTP API

일반적인 HTTP 요청을 통해 서버의 기능을 테스트하거나 단일 요청/응답이 필요한 경우 사용합니다.

### 1.1. 서버 상태 확인

#### `GET /health`

- **설명**: 서버가 정상적으로 실행 중인지 확인합니다.
- **요청**: 없음
- **성공 응답 (200 OK)**:
  - **Content-Type**: `application/json`
  - **Body**:
    ```json
    {
      "status": "ok"
    }
    ```

---

### 1.2. 통합 기능 API

#### `POST /ask`

- **설명**: 음성 파일을 업로드하여 STT(음성->텍스트), GPT(질의응답), TTS(텍스트->음성) 전체 파이프라인을 한 번에 실행하고 모든 결과를 반환받습니다.
- **요청**:
  - **Content-Type**: `multipart/form-data`
  - **Body**:
    - `file`: 오디오 파일 (e.g., `.wav`, `.mp3`, `.m4a`)
- **성공 응답 (200 OK)**:
  - **Content-Type**: `application/json`
  - **Body**:
    ```json
    {
      "transcribed_text": "음성인식으로 변환된 텍스트입니다.",
      "gpt_response_text": "GPT Agent가 생성한 답변 텍스트입니다.",
      "audio_content": "R1d..."
    }
    ```
    - `transcribed_text` (string): STT 결과 텍스트
    - `gpt_response_text` (string): GPT 답변 텍스트
    - `audio_content` (string): GPT 답변을 음성으로 변환한 WAV 데이터 (Base64 인코딩)

---

### 1.3. 개별 기능 테스트 API

각 모듈(STT, GPT, TTS)을 독립적으로 테스트하기 위한 API입니다.

#### `POST /test/stt`

- **설명**: 음성 파일을 텍스트로 변환합니다. (STT)
- **요청**:
  - **Content-Type**: `multipart/form-data`
  - **Body**:
    - `file`: 오디오 파일
- **성공 응답 (200 OK)**:
  - **Content-Type**: `application/json`
  - **Body**:
    ```json
    {
      "text": "음성인식으로 변환된 텍스트입니다."
    }
    ```

#### `POST /test/gpt`

- **설명**: 텍스트로 질문하고 GPT Agent의 답변을 텍스트로 받습니다. (GPT)
- **요청**:
  - **Content-Type**: `application/json`
  - **Body**:
    ```json
    {
      "text": "GPT에게 보낼 질문 텍스트"
    }
    ```
- **성공 응답 (200 OK)**:
  - **Content-Type**: `application/json`
  - **Body**:
    ```json
    {
      "response": "GPT Agent가 생성한 답변 텍스트입니다."
    }
    ```

#### `POST /test/tts`

- **설명**: 텍스트를 음성(WAV)으로 변환하여 바이너리 데이터를 직접 받습니다. (TTS)
- **요청**:
  - **Content-Type**: `application/json`
  - **Body**:
    ```json
    {
      "text": "음성으로 변환할 텍스트",
      "speaking_rate": 0.9,
      "pitch": -2.0,
      "voice_name": "ko-KR-Wavenet-D"
    }
    ```
    - `speaking_rate` (float, optional): 말하기 속도 (0.25 ~ 4.0)
    - `pitch` (float, optional): 음높이 (-20.0 ~ 20.0)
    - `voice_name` (string, optional): Google TTS 음성 모델 이름
- **성공 응답 (200 OK)**:
  - **Content-Type**: `audio/wav`
  - **Body**: 순수 WAV 바이너리 데이터

---

## 2. WebSocket API

실시간 양방향 통신이 필요할 때 사용합니다. 클라이언트는 하나의 WebSocket 연결을 통해 모든 기능을 수행할 수 있습니다.

**연결 주소**: `ws://<서버_주소>:<포트>/ws` (예: `ws://127.0.0.1:8000/ws`)

### 2.1. 통신 방식

클라이언트와 서버는 **JSON 형식의 텍스트 메시지** 또는 **순수 바이너리 오디오 데이터**를 주고받습니다.

- **JSON 메시지**: 모든 요청과 응답은 `type` 필드를 필수로 포함하여 메시지 종류를 구분합니다.
- **바이너리 메시지**: 클라이언트가 서버로 음성 데이터를 보낼 때 사용됩니다. (실시간 음성 전송)

### 2.2. 클라이언트 -> 서버 메시지 (C->S)

#### 1) 음성 질문 (JSON + Base64)

- **설명**: 녹음된 오디오를 Base64로 인코딩하여 `ask` 타입으로 전송합니다. STT -> GPT -> TTS 파이프라인이 실행됩니다.
- **메시지 형식**:
  ```json
  {
    "type": "ask",
    "audio_data": "R1d..."
  }
  ```
  - `audio_data` (string): 오디오 파일의 내용을 Base64로 인코딩한 문자열

#### 2) 음성 질문 (Binary)

- **설명**: 녹음된 오디오를 순수 바이너리 데이터로 전송합니다. STT -> GPT -> TTS 파이프라인이 실행됩니다. JSON 방식보다 효율적입니다.
- **메시지 형식**: 오디오 Blob/Binary 데이터 자체를 전송

#### 3) 개별 기능 테스트 (STT, GPT, TTS)

- **STT 요청**:
  ```json
  {
    "type": "stt",
    "audio_data": "R1d..."
  }
  ```
- **GPT 요청**:
  ```json
  {
    "type": "gpt",
    "text": "GPT에게 보낼 질문 텍스트"
  }
  ```
- **TTS 요청**:
  ```json
  {
    "type": "tts",
    "text": "음성으로 변환할 텍스트"
  }
  ```

#### 4) TTS 파라미터 설정

- **설명**: 현재 WebSocket 연결에 대한 TTS 설정을 변경합니다. 한 번 설정하면 연결이 끊길 때까지 유지됩니다.
- **메시지 형식**:
  ```json
  {
    "type": "config",
    "voice_name": "ko-KR-Wavenet-A",
    "speaking_rate": 1.0,
    "pitch": 0.0
  }
  ```
  - 모든 파라미터는 선택적(optional)이며, 보낸 파라미터만 업데이트됩니다.

---

### 2.3. 서버 -> 클라이언트 메시지 (S->C)

서버는 처리 단계별로 중간 결과 및 최종 결과를 클라이언트로 전송합니다.

#### 1) STT 결과

- **설명**: 음성 인식이 완료되었을 때 전송됩니다.
- **메시지 형식**:
  ```json
  {
    "type": "transcription",
    "text": "음성인식으로 변환된 텍스트입니다.",
    "status": "success"
  }
  ```
  - `ask` 또는 `stt` 요청에 대한 응답입니다. `stt` 요청의 경우 `type`이 `stt_result`일 수 있습니다.

#### 2) GPT 응답

- **설명**: GPT Agent의 텍스트 답변이 생성되었을 때 전송됩니다.
- **메시지 형식**:
  ```json
  {
    "type": "agent_response",
    "text": "GPT Agent가 생성한 답변 텍스트입니다.",
    "status": "success"
  }
  ```
  - `ask` 또는 `gpt` 요청에 대한 응답입니다. `gpt` 요청의 경우 `type`이 `gpt_result`일 수 있습니다.

#### 3) TTS 결과 (음성 데이터)

- **설명**: TTS 음성 생성이 완료되었을 때 전송됩니다.
- **메시지 형식**:
  ```json
  {
    "type": "audio_data",
    "format": "wav",
    "encoding": "base64",
    "data": "R1d...",
    "generation_time": 1.23,
    "text": "음성으로 변환된 텍스트",
    "tts_provider": "google"
  }
  ```
  - `data` (string): 생성된 WAV 음성 데이터 (Base64 인코딩)
  - `ask` 또는 `tts` 요청에 대한 응답입니다. `tts` 요청의 경우 `type`이 `tts_result`일 수 있습니다.

#### 4) 파이프라인 완료

- **설명**: `ask` 또는 바이너리 음성 요청에 대한 모든 파이프라인이 성공적으로 완료되었을 때 전송됩니다.
- **메시지 형식**:
  ```json
  {
    "type": "completed",
    "message": "ASK 프로세스가 성공적으로 완료되었습니다."
  }
  ```

#### 5) 설정 변경 확인

- **설명**: `config` 요청에 대한 확인 응답입니다.
- **메시지 형식**:
  ```json
  {
    "type": "config_ack",
    "voice_name": "ko-KR-Wavenet-A",
    "speaking_rate": 1.0,
    "pitch": 0.0,
    "message": "클라이언트 설정이 업데이트되었습니다."
  }
  ```

#### 6) 오류 응답

- **설명**: 처리 중 오류가 발생했을 때 전송됩니다.
- **메시지 형식**:
  ```json
  {
    "type": "error",
    "message": "오류에 대한 상세 설명입니다."
  }
  ```

---

## 3. WebSocket 통신 흐름 예시 (음성 질문)

1.  **클라이언트**: WebSocket 연결 (`/ws`)
2.  **클라이언트**: (선택) TTS 설정을 위해 `config` 메시지 전송
3.  **서버**: `config_ack` 메시지로 응답
4.  **클라이언트**: `ask` (JSON+Base64) 또는 `Binary` 오디오 데이터 전송
5.  **서버**: `transcription` 메시지 전송 (STT 결과)
6.  **서버**: `agent_response` 메시지 전송 (GPT 결과)
7.  **서버**: `audio_data` 메시지 전송 (TTS 결과)
8.  **서버**: `completed` 메시지 전송 (프로세스 완료)