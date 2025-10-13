# Voice Server API 가이드 (for Unreal Engine)

이 문서는 Unreal Engine 클라이언트에서 Voice Server의 API를 호출하는 방법을 안내합니다. 각 API는 표준 HTTP 요청을 따르며, 파일 전송이 필요한 경우 `multipart/form-data` 형식을 사용합니다.

## 1. 서버 상태 확인 (Health Check)

서버가 정상적으로 실행 중인지 확인합니다.

-   **Endpoint:** `/health`
-   **Method:** `GET`
-   **Request:** 없음
-   **Success Response (200 OK):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "status": "ok"
        }
        ```

---

## 2. 음성 인식 (Speech-to-Text)

사용자의 음성 파일(예: `.wav`)을 서버로 보내 텍스트로 변환합니다.

-   **Endpoint:** `/stt`
-   **Method:** `POST`
-   **Request:**
    -   **Content-Type:** `multipart/form-data`
    -   **Parameters:**

| 필드명 | 타입 | 필수 여부 | 설명                                  |
| :----- | :--- | :-------- | :------------------------------------ |
| `file` | File | **Yes**   | 텍스트로 변환할 음성 파일 (e.g., .wav) |

-   **Success Response (200 OK):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "text": "음성 파일에서 변환된 텍스트입니다."
        }
        ```
-   **Error Response (500 Internal Server Error):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "detail": "STT processing failed: <오류 상세 메시지>"
        }
        ```

---

## 3. 음성 합성 (Text-to-Speech) - 참조 음성 사용

특정 목소리(참조 음성)를 기반으로 주어진 텍스트를 음성 파일로 생성합니다.

-   **Endpoint:** `/fish/tts`
-   **Method:** `POST`
-   **Request:**
    -   **Content-Type:** `multipart/form-data`
    -   **Parameters:**

| 필드명            | 타입   | 필수 여부 | 설명                                                              |
| :---------------- | :----- | :-------- | :---------------------------------------------------------------- |
| `text`            | String | **Yes**   | 음성으로 합성할 텍스트                                            |
| `reference_audio` | File   | **Yes**   | 목소리 톤의 기반이 될 참조 음성 파일 (.wav 권장)                  |
| `reference_text`  | String | No        | 참조 음성의 텍스트 내용 (선택 사항, 제공 시 합성 정확도 향상) |

-   **Success Response (200 OK):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "message": "Fish-Speech TTS V2 audio generated successfully.",
          "file_path": "./temp/fish_tts_v2_20231027_123456.wav",
          "requested_text": "합성을 요청한 텍스트입니다.",
          "reference_audio_used": "ref_20231027_123456.wav",
          "generation_time": 4.58,
          "version": "v2-worker"
        }
        ```
    > **Note:** `file_path`는 서버 내부 경로입니다. 클라이언트에서는 이 경로를 직접 사용할 수 없으며, 별도의 파일 다운로드 API가 필요하다면 추가 구현이 필요합니다. (현재는 파일 경로만 반환)

-   **Error Response (500 Internal Server Error):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "detail": "TTS processing failed: <오류 상세 메시지>"
        }
        ```

---

## 4. 음성 합성 (Text-to-Speech) - 기본 목소리 사용

서버에 설정된 기본 목소리로 텍스트를 음성 파일로 생성합니다. (참조 음성 불필요)

-   **Endpoint:** `/fish/tts-simple`
-   **Method:** `POST`
-   **Request:**
    -   **Content-Type:** `application/x-www-form-urlencoded` 또는 `application/json`
    -   **Parameters:**

| 필드명 | 타입   | 필수 여부 | 설명                   |
| :----- | :----- | :-------- | :--------------------- |
| `text` | String | **Yes**   | 음성으로 합성할 텍스트 |

-   **Success Response (200 OK):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "message": "Fish-Speech TTS V2 audio generated successfully (no reference audio).",
          "file_path": "./temp/fish_tts_v2_simple_20231027_123457.wav",
          "requested_text": "합성을 요청한 텍스트입니다.",
          "generation_time": 3.12,
          "version": "v2-worker"
        }
        ```

-   **Error Response (500 Internal Server Error):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "detail": "Simple TTS processing failed: <오류 상세 메시지>"
        }
        ```


## 4. 음성 합성 (Text-to-Speech) - ASK라는 API를 추가할꺼다                                                                                                                                                                                                                                                 │                              
│                                                                                                                                                                                                                                                                            │                              
/ask 는 언리얼쪽에서 녹음한 음성데이터를 서버로 넘긴다                                                                                                                                                                                                                    │                              
│                                                                                                                                                                                                                                                                            │                              
│   이때 서버는 넘어오면 Whisper를 통해서 String 화 하고                                                                                                                                                                                                                 │                              
│   언리얼쪽으로 메세지 값을 돌려주고,                                                                                                                                                                                                                                       │                              
│                                                                                                                                                                                                                                                                            │                              
│   자동으로 서버에서는 String화 한 데이터를 Fish-speech를 통해서 음성파일로 생성해서                                                                                                                                                                                        │                              
│   다시 소켓을 통해서 언리얼에 전송받아서 출력할꺼다.

---

## 5. 음성 질의 응답 (Voice Q&A)

사용자의 음성 파일을 서버로 보내 텍스트로 변환하고, 변환된 텍스트를 클라이언트에 반환합니다. 이후 서버는 해당 텍스트를 기반으로 음성 파일을 생성하여 소켓을 통해 클라이언트에 전송합니다.

-   **Endpoint:** `/ask`
-   **Method:** `POST`
-   **Request:**
    -   **Content-Type:** `multipart/form-data`
    -   **Parameters:**

| 필드명 | 타입 | 필수 여부 | 설명                                  |
| :----- | :--- | :-------- | :------------------------------------ |
| `file` | File | **Yes**   | 텍스트로 변환할 음성 파일 (e.g., .wav) |

-   **Success Response (200 OK):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "text": "음성 파일에서 변환된 텍스트입니다."
        }
        ```
-   **Error Response (500 Internal Server Error):**
    -   **Content-Type:** `application/json`
    -   **Body:**
        ```json
        {
          "detail": "Voice Q&A processing failed: <오류 상세 메시지>"
        }
        ```
    > **Note:** 음성 파일 응답은 별도의 WebSocket 연결을 통해 비동기적으로 전송됩니다. 클라이언트는 `/ask` 요청 후 WebSocket을 통해 음성 파일을 수신할 준비가 되어 있어야 합니다.

---