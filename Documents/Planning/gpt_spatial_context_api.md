# GPT 에이전트 공간 컨텍스트 연동 가이드

다산 음성 챗봇이 게임 엔진의 위치·오브젝트 정보를 활용해 더 정확한 답변을 제공할 수 있도록 HTTP/웹소켓 API 포맷이 확장되었습니다. 이 문서는 서버와 통신하는 클라이언트(게임 엔진, 모니터링 툴 등)가 구현해야 할 요청 구조와 처리 흐름을 정리합니다.

---

## 1. 핵심 변경 사항
- `POST /test/gpt` 엔드포인트가 `user_query` 와 공간 컨텍스트(`context`) JSON을 함께 받도록 확장되었습니다.
- STT→GPT 파이프라인과 WebSocket `gpt` 메시지는 마지막으로 저장된 컨텍스트를 자동 재사용합니다.

---

## 2. HTTP API (단발성 테스트 용도)

### 2.1 요청 포맷
```
POST /test/gpt
Content-Type: application/json
```
```json
{
  "user_query": "지금 여기 옆에는 뭐가 있느냐",
  "context": {
    "current_location": {
      "name": "운한각",
      "x": 100.2,
      "y": 33.5,
      "z": -12.7
    },
    "focused_object": {
      "name": "신풍루",
      "x": 120.5,
      "y": 40.1,
      "z": -50.3
    },
    "nearby_buildings": [
      { "name": "별주", "distance": 15.3 },
      { "name": "집사청", "distance": 22.1 },
      { "name": "북군영", "distance": 35.7 }
    ]
  }
}
```

| 필드 | 필수 | 설명 |
|------|------|------|
| `user_query` | ✅ | 왕이 물은 최종 질문. 과거 호환을 위해 `text` 필드도 허용되지만 앞으로는 `user_query` 사용을 권장합니다. |
| `context.current_location` | ⛔ | 전하의 현재 위치 이름 및 좌표. 좌표는 선택입니다. |
| `context.focused_object` | ⛔ | 전하가 주시 중인 대상. `저것`, `저기` 등의 지시어 해석에 사용됩니다. |
| `context.nearby_buildings` / `context.nearest_buildings` | ⛔ | 주변 건물 배열. 이름과 거리(m) 정보를 제공하면 인접 순으로 설명에 활용됩니다. |

> ⚠️ 전달되는 이름/거리 값은 그대로 답변에 사용되므로, 실제 명칭을 지정해 주세요. 모델은 새로운 좌표 계산을 수행하지 않습니다.

### 2.2 응답 예시
```json
{
  "response": "신풍루 오른편에는 별주가 가장 가깝사옵고 이어 집사청과 북군영이 있사옵니다"
}
```

### 2.3 에러 응답
| HTTP 상태 | 원인 | 예시 메시지 |
|-----------|------|-------------|
| 422 | `user_query` / `text` 미제공 | `"user_query 또는 text 필드를 통해 질문을 전달해 주세요."` |
| 503 | GPT Agent 미초기화 | `"GPT Agent 서비스가 초기화되어 있지 않습니다."` |
| 500 | 응답 생성 중 오류 | OpenAI 호출 실패, 검색 오류 등 |

---

### 3.2 GPT 텍스트 요청
```json
{
  "type": "gpt",
  "user_query": "지금 여기 옆에는 뭐가 있느냐"
}
```

- `user_query` 미전달 시 400 오류 (`"GPT 요청에는 'user_query' 또는 'text' 필드가 필요합니다."`)
- `context` 필드를 함께 보내면 즉시 갱신 후 사용됩니다. 생략하면 세션에 저장된 컨텍스트가 사용됩니다.

**서버 응답**
```json
{
  "type": "gpt_result",
  "text": "신풍루 오른편에는 별주가 가장 가깝사옵고 이어 집사청과 북군영이 있사옵니다",
  "status": "success"
}
```

### 3.3 음성 ASK (STT → GPT → TTS)
- `stop_recording` 이후 내부적으로 STT 결과가 GPT로 전달되며, 위에서 저장한 컨텍스트가 자동으로 병합됩니다.
- 별도의 JSON 조작 필요 없이 `context` 메시지 전송 → 음성 녹음/ASK 요청 순서를 지키면 됩니다.

**중간 이벤트**
| 타입 | 설명 |
|------|------|
| `transcription` | STT 텍스트와 메타데이터 |
| `agent_response` | GPT 답변 텍스트 |
| `audio_data` | TTS WAV(base64) |
| `completed` | ASK 파이프라인 종료 |
| `error` | 단계별 오류 메시지 |

---

## 4. 데이터 규칙 및 주의 사항

- 건물 이름/거리/좌표는 신뢰 가능한 값으로 제공해야 답변 품질이 올라갑니다.
- `nearby_buildings` 는 가까운 순서로 정렬해서 보내면, 다산이 그대로 “가장 가까움 → 그 다음” 순서로 설명합니다.
- 좌표 필드는 선택 사항이지만, 추후 시각화/디버깅 자료로 활용될 수 있습니다.
- 컨텍스트가 존재하지 않을 때 다산은 추가 추론을 하지 않고 “기록이 없다” 고 짧게 답합니다.
- HTTP/WS 모두 JSON 키는 snake_case (`current_location`, `focused_object`) 를 사용합니다.

---

## 5. 클라이언트 적용 체크리스트
1. **HTTP 단발성 호출**: 개발/디버깅 용도로 `POST /test/gpt` 요청을 업데이트한다.
2. **WebSocket 연결 초기화**: `/ws` 접속 후 `context` 메시지를 가장 먼저 전송한다.
3. **컨텍스트 유지**: 위치 변동·카메라 초점이 바뀔 때마다 `context` 메시지를 다시 보낸다.
4. **ASK 흐름**: `context_ack` 확인 → 음성 녹음 및 `ask` → 응답 수신 순서를 지킨다.
5. **오류 대비**: 422/500 응답, `error` 이벤트 수신 시 사용자 알림 및 재시도를 구현한다.
6. **로그/디버그**: 서버는 로그에 전달된 컨텍스트를 출력하므로, 문제 발생 시 서버 로그를 확인한다.

---

## 6. 참고 모듈
- `voice_server/gpt_agent_service.py`: Responses API 호출과 동적 컨텍스트 프롬프트 생성 로직.
- `voice_server/endpoints/http_router.py`: `POST /test/gpt` 처리.
- `voice_server/endpoints/websocket_router.py`: `context`, `gpt`, ASK 파이프라인 처리.
- `requirements.txt`: `httpx` 의존성이 추가되었습니다 (외부 검색용).

---