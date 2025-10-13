"""FastAPI 서버: NCP CLOVA Speech Recognition/Voice 테스트 엔드포인트.

이 모듈은 Unreal/서버 통합용으로 CSR(STT)과 CLOVA Voice(TTS) REST API를 프록시 호출한다.
환경 변수로 인증 키를 주입해야 하며, 테스트 목적의 `/testncp` 엔드포인트 및 모니터 페이지를 제공한다.
"""
from __future__ import annotations

import base64
import logging
import os
from pathlib import Path
from typing import Any, Dict

import httpx
from dotenv import load_dotenv
from fastapi import FastAPI, File, Form, HTTPException, UploadFile
from fastapi.responses import HTMLResponse, JSONResponse
from httpx import HTTPError

load_dotenv()

LOGGER = logging.getLogger("testncp")
logging.basicConfig(level=logging.INFO)

DEFAULT_STT_ENDPOINT = "https://naveropenapi.apigw.ntruss.com/recog/v1/stt"
DEFAULT_LANGUAGE = "ko-KR"

DEFAULT_TTS_ENDPOINT = "https://naveropenapi.apigw.ntruss.com/voice/v1/tts"
DEFAULT_TTS_SPEAKER = "nara"
DEFAULT_TTS_FORMAT = "wav"

MONITOR_TEMPLATE_PATH = Path(__file__).parent / "templates" / "testncp_monitor.html"


def _get_env(name: str) -> str:
    value = os.getenv(name)
    if not value:
        raise RuntimeError(f"환경 변수 {name} 값이 비어 있습니다.")
    return value


def build_stt_headers(client_id: str, client_secret: str) -> Dict[str, str]:
    return {
        "X-NCP-APIGW-API-KEY-ID": client_id,
        "X-NCP-APIGW-API-KEY": client_secret,
        "Content-Type": "application/octet-stream",
    }


def build_tts_headers(client_id: str, client_secret: str) -> Dict[str, str]:
    return {
        "X-NCP-APIGW-API-KEY-ID": client_id,
        "X-NCP-APIGW-API-KEY": client_secret,
        "Content-Type": "application/x-www-form-urlencoded; charset=utf-8",
    }


app = FastAPI(title="YiSan TestNCP Proxy", version="0.2.0")


@app.on_event("startup")
async def startup_event() -> None:
    LOGGER.info(
        "testncp_app startup - stt_endpoint=%s, tts_endpoint=%s",
        os.getenv("CSR_ENDPOINT_URL", DEFAULT_STT_ENDPOINT),
        os.getenv("TTS_ENDPOINT_URL", DEFAULT_TTS_ENDPOINT),
    )
    if not MONITOR_TEMPLATE_PATH.exists():
        LOGGER.warning("Monitor 템플릿이 누락되었습니다: %s", MONITOR_TEMPLATE_PATH)


async def _call_stt(
    payload: bytes,
    target_language: str,
    client_id: str,
    client_secret: str,
) -> Dict[str, Any]:
    endpoint = os.getenv("CSR_ENDPOINT_URL", DEFAULT_STT_ENDPOINT)
    headers = build_stt_headers(client_id, client_secret)
    params = {"lang": target_language}

    LOGGER.info(
        "[REQ][CSR] endpoint=%s, lang=%s, size=%d",
        endpoint,
        target_language,
        len(payload),
    )

    async with httpx.AsyncClient(timeout=httpx.Timeout(60.0, read=60.0)) as client:
        try:
            response = await client.post(endpoint, params=params, headers=headers, content=payload)
            response.raise_for_status()
        except HTTPError as exc:
            LOGGER.exception("CSR 호출 실패: %s", exc)
            detail = getattr(exc.response, "text", str(exc)) if hasattr(exc, "response") else str(exc)
            raise HTTPException(status_code=502, detail=f"CSR 호출 실패: {detail}") from exc

    try:
        data: Dict[str, Any] = response.json()
    except ValueError as exc:
        LOGGER.exception("CSR 응답 JSON 파싱 실패")
        raise HTTPException(status_code=502, detail="CSR 응답 파싱 실패") from exc

    LOGGER.info("[RES][CSR] keys=%s", list(data.keys()))
    return data


async def _call_tts(
    text: str,
    speaker: str,
    fmt: str,
    speed: str | None,
    client_id: str,
    client_secret: str,
) -> Dict[str, Any]:
    endpoint = os.getenv("TTS_ENDPOINT_URL", DEFAULT_TTS_ENDPOINT)
    headers = build_tts_headers(client_id, client_secret)

    payload = {"speaker": speaker, "text": text, "format": fmt}
    if speed:
        payload["speed"] = speed

    LOGGER.info(
        "[REQ][TTS] endpoint=%s, speaker=%s, format=%s, length=%d",
        endpoint,
        speaker,
        fmt,
        len(text),
    )

    async with httpx.AsyncClient(timeout=httpx.Timeout(60.0, read=60.0)) as client:
        try:
            response = await client.post(endpoint, headers=headers, data=payload)
            response.raise_for_status()
        except HTTPError as exc:
            LOGGER.exception("TTS 호출 실패: %s", exc)
            detail = getattr(exc.response, "text", str(exc)) if hasattr(exc, "response") else str(exc)
            raise HTTPException(status_code=502, detail=f"TTS 호출 실패: {detail}") from exc

    audio_bytes = response.content
    content_type = response.headers.get("Content-Type", "audio/mpeg")
    encoded = base64.b64encode(audio_bytes).decode("ascii")

    LOGGER.info("[RES][TTS] bytes=%d, content_type=%s", len(audio_bytes), content_type)
    return {
        "audio_base64": encoded,
        "content_type": content_type,
    }


@app.post("/testncp")
async def post_testncp(
    service: str = Form("stt"),
    file: UploadFile | None = File(default=None),
    language: str | None = Form(default=None),
    text: str | None = Form(default=None),
    speaker: str | None = Form(default=None),
    audio_format: str | None = Form(default=None),
    speed: str | None = Form(default=None),
) -> JSONResponse:
    """CSR(STT) 또는 CLOVA Voice(TTS) 테스트 호출."""

    try:
        client_id = _get_env("NCP_CLIENT_ID")
        client_secret = _get_env("NCP_CLIENT_SECRET")
    except RuntimeError as exc:  # pragma: no cover - 환경 변수 누락 대응
        LOGGER.error("환경 변수 오류: %s", exc)
        raise HTTPException(status_code=500, detail=str(exc)) from exc

    normalized_service = service.lower()

    if normalized_service == "stt":
        if file is None:
            raise HTTPException(status_code=400, detail="STT 호출에는 음성 파일이 필요합니다.")

        try:
            payload = await file.read()
        finally:
            await file.close()

        if not payload:
            raise HTTPException(status_code=400, detail="빈 파일입니다.")

        target_language = language or os.getenv("CSR_LANGUAGE", DEFAULT_LANGUAGE)
        stt_data = await _call_stt(payload, target_language, client_id, client_secret)
        return JSONResponse({
            "mode": "stt",
            "language": target_language,
            "text": stt_data.get("text"),
            "raw": stt_data,
        })

    if normalized_service == "tts":
        if not text or not text.strip():
            raise HTTPException(status_code=400, detail="TTS 호출에는 변환할 문장이 필요합니다.")

        speaker_name = speaker or os.getenv("TTS_SPEAKER", DEFAULT_TTS_SPEAKER)
        fmt = audio_format or os.getenv("TTS_FORMAT", DEFAULT_TTS_FORMAT)
        speed_value = speed or os.getenv("TTS_SPEED")

        tts_data = await _call_tts(text.strip(), speaker_name, fmt, speed_value, client_id, client_secret)
        return JSONResponse({
            "mode": "tts",
            "speaker": speaker_name,
            "format": fmt,
            "content_type": tts_data["content_type"],
            "audio_base64": tts_data["audio_base64"],
        })

    raise HTTPException(status_code=400, detail="service 파라미터는 'stt' 또는 'tts'만 허용합니다.")


@app.get("/monitor", response_class=HTMLResponse, tags=["monitor"])
async def monitor_page() -> HTMLResponse:
    """STT/TTS 테스트용 간이 모니터 페이지."""
    try:
        html = MONITOR_TEMPLATE_PATH.read_text(encoding="utf-8")
    except FileNotFoundError as exc:
        LOGGER.error("Monitor 템플릿을 찾을 수 없습니다: %s", exc)
        raise HTTPException(status_code=500, detail="Monitor 템플릿이 존재하지 않습니다.") from exc
    return HTMLResponse(content=html)


@app.get("/health", tags=["system"])
async def health() -> Dict[str, str]:
    return {"status": "ok"}


if __name__ == "__main__":
    import uvicorn

    uvicorn.run("testncp_app:app", host="0.0.0.0", port=4010, reload=False)
