# NCP CLOVA Speech Recognition(STT) & Voice(TTS) 적용 가이드

## 1) 단계별 체크리스트

### 1단계. 초기 세팅
1. [ ] https://www.ncloud.com 에서 NCP 계정 가입 또는 로그인 상태 확인.
2. [ ] 콘솔 접속 후 프로젝트(또는 `Solution`)가 생성되어 있는지 확인하고, 없다면 새로 생성.
3. [ ] 콘솔 우측 상단 리전이 `KR`(한국)으로 지정되어 있는지 확인. 필요 시 리전 변경.
4. [ ] 과금/결제 수단 등록 여부 확인. 신규 계정의 경우 3개월 무료 크레딧(최대 30만 원) 잔여량 확인.

### 2단계. API 활성화 및 키 발급
1. [ ] 콘솔 메뉴에서 **AI·NAVER CLOVA > Speech Recognition** 서비스 진입 후 `사용 신청`.
2. [ ] 동일 콘솔에서 **AI·NAVER CLOVA > Voice**(Speech Synthesis) 서비스도 함께 활성화.
3. [ ] 프로젝트 > **인증키 관리** 또는 **API 인증 정보** 메뉴에서 Access Key(Client ID), Secret Key(Client Secret) 발급.
4. [ ] 키 발급 시 필요한 역할/권한(예: `CMS Role`)과 사용 제한(IP 제한, 만료일) 옵션 확인 후 설정.
5. [ ] 발급 키는 `Server` 혹은 `Application` 용도로 구분해 저장하고, 외부 노출 금지.

### 3단계. 환경 변수 및 설정 파일 정리
1. [ ] 서버/개발 PC `.env` 파일 혹은 시스템 환경 변수에 CSR/TTS 관련 키/설정을 저장.
2. [ ] 언리얼/서버 공용 설정 저장소(예: `Config/`)에는 민감 정보 대신 참조 키 사용.
3. [ ] Git 저장소에는 실제 키가 포함되지 않도록 `.gitignore` 적용.

### 4단계. 호출 방식 정의
1. [ ] REST(STT): `POST https://naveropenapi.apigw.ntruss.com/recog/v1/stt`
   - 헤더: `X-NCP-APIGW-API-KEY-ID`, `X-NCP-APIGW-API-KEY`, `Content-Type: application/octet-stream`.
   - 쿼리: `lang=ko-KR` 등.
   - 바디: PCM/WAV 바이너리 (16kHz/16bit/mono 권장).
   - 응답: `{ "text": "..." }`.
2. [ ] REST(TTS): `POST https://naveropenapi.apigw.ntruss.com/voice/v1/tts`
   - 헤더: `X-NCP-APIGW-API-KEY-ID`, `X-NCP-APIGW-API-KEY`, `Content-Type: application/x-www-form-urlencoded`.
   - 폼 데이터: `speaker=nara`, `format=wav|mp3`, `speed=-5~5`, `text=...`.
   - 응답: 바이너리 음성 데이터(`audio/wav`, `audio/mpeg` 등).
3. [ ] Streaming(WebSocket) API: `wss://naveropenapi.apigw.ntruss.com/recog/v1/ws/stt`
   - WebSocket 프레임에 100~200ms 단위 PCM chunk를 전송.
   - 초기 프레임에 인증 정보(JSON) 전달 필요.
   - 응답 프레임으로 중간/최종 인식 결과(`message`, `state`) 수신.
4. [ ] 파일 업로드 시 WAV 헤더 제거 후 순수 PCM 전송 가능. 언리얼 마이크 캡처는 16-bit PCM 변환 필요.

### 5단계. 샘플 코드/서버 준비
1. [ ] Python FastAPI 서버(`Tools/Server/testncp_app.py`) 배치.
2. [ ] `/testncp` 엔드포인트에서 `service=stt|tts` 모드 지원. WAV 업로드(STT) 또는 텍스트 입력(TTS) 후 결과 확인.
3. [ ] `/monitor` 페이지에서 브라우저 기반 STT/TTS 수동 검증 가능.
4. [ ] Uvicorn(포트 4010)으로 로컬 실행 후 언리얼 HTTP 모듈 또는 파이썬 스크립트에서 테스트.

### 6단계. 테스트 및 검증 루틴
1. [ ] 샘플 WAV 파일을 `/testncp`(service=stt) 또는 `/monitor` STT 섹션에서 업로드하여 인식 결과 확인.
2. [ ] 임의 문장을 `/testncp`(service=tts) 또는 `/monitor` TTS 섹션에서 합성 후 오디오 플레이어 재생 확인.
3. [ ] 잘못된 키/언어 코드/포맷을 각각 시도하여 에러 메시지 및 로그를 검증.
4. [ ] Whisper 대비 테스트: 동일 WAV 파일로 Whisper 서버 호출 결과와 정확도/응답시간 비교 저장.

### 7단계. 비용 및 제한 사항 확인
1. [ ] CSR 월 무료 10시간 제공 (600분). 1분 음성=약 1MB PCM 기준 -> 월 600MB까지 무료 추정.
2. [ ] CLOVA Voice(TTS)는 월 무료 1백만자(프로모션 상이) 수준 제공. 초과 시 문자당 0.0045원(예시) 과금, 최신 단가 확인.
3. [ ] 초과 시 시간당 과금(STT)·문자 과금(TTS)이 동시에 적용되므로 서비스별 모니터링 필요.
4. [ ] 기본 요청 제한: 초당 최대 10 RPS 권장, 동시 연결(WebSocket) 20개 내 권장. TTS는 요청 길이에 따라 처리 지연 발생 가능.
5. [ ] 서비스 상태/지연 발생 시 NCP 상태 페이지 확인.

---

## 2) 환경 변수 / 설정 파일 예시

`.env` 예시:
```bash
NCP_CLIENT_ID=your_access_key
NCP_CLIENT_SECRET=your_secret_key
CSR_LANGUAGE=ko-KR
CSR_ENDPOINT_URL=https://naveropenapi.apigw.ntruss.com/recog/v1/stt
CSR_STREAM_ENDPOINT=wss://naveropenapi.apigw.ntruss.com/recog/v1/ws/stt
CSR_SAMPLE_RATE=16000
TTS_ENDPOINT_URL=https://naveropenapi.apigw.ntruss.com/voice/v1/tts
TTS_SPEAKER=nara
TTS_FORMAT=wav
TTS_SPEED=0
```

언리얼 `Config/DefaultEngine.ini` 내 레퍼런스(민감 값 제외):
```ini
[/Script/YiSan.VoiceIntegrationSettings]
DefaultLanguage="ko-KR"
CsrRestUrl="https://naveropenapi.apigw.ntruss.com/recog/v1/stt"
ProxyServerUrl="http://localhost:4010/testncp"
MonitorPageUrl="http://localhost:4010/monitor"
```

---

## 3) FastAPI 샘플 코드

`Tools/Server/testncp_app.py` (핵심 발췌):
```python
@app.post("/testncp")
async def post_testncp(
    service: str = Form("stt"),
    file: UploadFile | None = File(default=None),
    language: str | None = Form(default=None),
    text: str | None = Form(default=None),
    speaker: str | None = Form(default=None),
    audio_format: str | None = Form(default=None),
    speed: str | None = Form(default=None),
):
    """CSR(STT) 또는 CLOVA Voice(TTS) 테스트 호출."""
    client_id = _get_env("NCP_CLIENT_ID")
    client_secret = _get_env("NCP_CLIENT_SECRET")
    if service == "stt":
        if not file:
            raise HTTPException(status_code=400, detail="STT 호출에는 음성 파일이 필요합니다.")
        payload = await file.read()
        language = language or os.getenv("CSR_LANGUAGE", "ko-KR")
        data = await _call_stt(payload, language, client_id, client_secret)
        return JSONResponse({"mode": "stt", "language": language, "text": data.get("text"), "raw": data})
    if service == "tts":
        if not text:
            raise HTTPException(status_code=400, detail="TTS 호출에는 변환할 문장이 필요합니다.")
        speaker = speaker or os.getenv("TTS_SPEAKER", "nara")
        fmt = audio_format or os.getenv("TTS_FORMAT", "wav")
        data = await _call_tts(text.strip(), speaker, fmt, speed or os.getenv("TTS_SPEED"), client_id, client_secret)
        return JSONResponse({"mode": "tts", "speaker": speaker, "format": fmt, "audio_base64": data["audio_base64"], "content_type": data["content_type"]})
    raise HTTPException(status_code=400, detail="service 파라미터는 'stt' 또는 'tts'만 허용합니다.")
```

> 의존성: `fastapi`, `uvicorn[standard]`, `httpx`, `python-dotenv`. (모니터 페이지는 별도 프론트 의존성 없음) `Tools/Server/requirements.txt` 참고.

실행:
```bash
cd Tools/Server
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn testncp_app:app --host 0.0.0.0 --port 4010
```

---

## 4) REST 호출 예시

### STT 요청 (curl)
```bash
curl -X POST "http://localhost:4010/testncp" \
     -H "Content-Type: multipart/form-data" \
     -F "service=stt" \
     -F "language=ko-KR" \
     -F "file=@sample.wav"
```

### STT 기대 응답
```json
{
  "mode": "stt",
  "language": "ko-KR",
  "text": "안녕하세요, 테스트 음성입니다.",
  "raw": {
    "text": "안녕하세요, 테스트 음성입니다.",
    "confidence": 0.93,
    "message": "SUCCESS"
  }
}
```

### TTS 요청 (curl)
```bash
curl -X POST "http://localhost:4010/testncp" \
     -H "Content-Type: multipart/form-data" \
     -F "service=tts" \
     -F "text=정조대왕 프로젝트 음성 합성 테스트" \
     -F "speaker=nara" \
     -F "audio_format=mp3"
```

### TTS 기대 응답
```json
{
  "mode": "tts",
  "speaker": "nara",
  "format": "mp3",
  "content_type": "audio/mpeg",
  "audio_base64": "SUQzBAAAAAAAI1RTU0UAAA..."  // Base64 인코딩된 음성(일부 생략)
}
```

### 대표 오류 응답
```json
{
  "detail": "CSR 호출 실패: {\"msg\":\"Invalid Signature\"}"
}
```

---

## 5) 이후 확장 작업 로드맵

- **언리얼 연동**: `FHttpModule` 기반 POST 요청 래퍼 작성 후 `/testncp` 호출. `AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask)`로 비동기 처리.
- **WebSocket 스트리밍**: Unreal `FWebSocketsModule` 또는 Python `websockets` 라이브러리로 `wss://.../ws/stt` 연결. PCM chunk 전송 로직 구현.
- **서버 확장**: `/testncp` 외 `/csr/stream` 라우트 추가하여 WebSocket 프록시 구현 고려, `/monitor`에 실시간 로그 패널 추가 검토.
- **관측성**: FastAPI `logging` 확장 및 Prometheus exporter 적용. 실패율 30% 이상 시 FailureDigest 로그 남기는 정책 준비.
- **키 로테이션**: NCP 콘솔에서 주기적으로 Secret 교체 후 `.env` 업데이트 자동화 스크립트 작성.
- **Whisper 비교 벤치마크**: 동일 입력에 대해 응답 지연, 정확도, 비용 지표 수집 후 `Documents/Server/` 내 리포트 추가.
- **보안**: HTTPS Reverse Proxy(Nginx) 구성 및 IP 허용 목록 설정. 키는 Vault/Secret Manager 연동 고려.

---

## 참고 자료
- NCP 공식 문서: [CLOVA Speech Recognition Guide](https://api.ncloud-docs.com/docs/en/ai-csr-overview)
- NCP 공식 문서: [CLOVA Voice Guide](https://api.ncloud-docs.com/docs/en/ai-voice-tts-overview)
- 기존 서버 API 가이드: `Documents/server_api.md`
