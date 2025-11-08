# 음성 프롬프트 워크플로 계획 (2025-10-02)

## 목표
- Push-to-talk 오디오를 캡처하고, Google STT를 통해 텍스트로 변환한 후, GPT/GPT 유사 파이프라인에 전달합니다.
- 사용자가 직접 텍스트를 입력하고 Enter를 누를 때, 동일한 음성 서비스(TTS)를 재사용하는 텍스트 입력 UX를 제공합니다.
- `UVoiceSystem`의 시도/결과 로깅을 통해 관찰 가능성을 유지합니다.

## 제안 흐름
1. **플레이어 입력 레이어**
   - `APlayerActor`가 스페이스바 (누름/뗌) 입력을 받아 STT를 위해 `UVoiceCaptureComponent`로 전달합니다.
   - `UMainWdget`은 `UEditableTextBox`(`VoiceInputBox`)를 호스팅합니다. Enter 입력 시, 텍스트를 로그로 남기고, 입력창을 비운 뒤, `UVoiceCaptureComponent::RequestSynthesis` (TTS 경로)를 호출합니다.

2. **음성 세션 레이어**
   - `UVoiceSystem`은 상관관계 ID, 대화 내용 기록, 관찰 가능성(`PRINTLOG`, FailureDigest)을 관리합니다.
   - 블루프린트를 통해 UI/HUD에 데이터를 공급하는 이벤트를 노출합니다: `OnVoiceTranscriptReceived`, `OnVoiceResponseReceived`.

3. **캡처 및 네트워크 레이어**
   - `UVoiceCaptureComponent`
     - 스페이스바 시작: `StartAudioCapture()`; 중지: 버퍼를 비우고 `RequestSpeechToText`를 호출합니다.
     - STT 성공 시: `VoiceSystem->SubmitTranscription`을 호출하고, 선택적으로 `RequestTextToSpeech`를 통해 TTS를 요청합니다.
     - TTS 결과는 블루프린트 델리게이트를 트리거하고 로컬에서 사운드를 재생합니다.
   - `UGoogleSpeechService`
     - `UGoogleSpeechServiceSettings` (API 키, 언어, 음성 이름)를 통해 설정됩니다.
     - REST 요청은 `NETWORK_LOG`로 계측됩니다.

4. **콘솔/디버그 레이어**
   - `UYiSanVoiceConsoleSubsystem`은 빠른 스크립팅 및 QA를 위해 `yisan.tts <text>` 명령을 등록합니다.

## 협업 및 반복 단계
1. Google API 접근을 확인하고 `Project Settings > Google Speech Service` (API 키, 언어, 음성)를 채웁니다.
2. `UMainWdget`의 블루프린트 서브클래스를 생성 또는 업데이트하여 UI 이벤트를 바인딩하고 HUD에 추가합니다.
3. `OnVoiceTranscriptReceived`를 UI 로그/채팅 패널에 연결하고, 선택적으로 GPT 백엔드로 전송합니다.
4. 병합 전에 콘솔 명령어와 자동화된 테스트(스페이스바, TTS 시뮬레이션)를 통해 QA를 진행합니다.
5. 실패 분석을 위해 `PRINTLOG` 출력(`LogCoffee`)과 `NETWORK_LOG`에서 원격 측정 데이터를 수집합니다.

## 미해결 질문
- TTS 재생을 선택 사항(음소거)으로 만들고 위젯 토글로 제어해야 할까요?
- 여러 TTS 응답이 빠르게 도착할 경우, 대기열 재생이 필요할까요?
- 향후 과제: `SubmitTranscription` 성공 후 GPT 호출을 통합하고 `NotifyResponseReady`에 데이터를 제공합니다.