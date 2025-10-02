# UVoiceSystem

`UVoiceSystem`은 실시간 STT/GPT 스택이 연결되기 전 캐릭터의 음성 캡처 세션을 관리합니다.

- 관찰 가능성 파이프라인을 위해 상관관계 ID와 대화 기록을 제공합니다.
- 기존 캐릭터 컴포넌트(`UStatSystem`, `USightSystem`, `UHitStopSystem`)와 연동하여 로직 중복 없이 더 풍부한 컨텍스트를 노출합니다.
- `PRINTLOG`를 통해 시도/결과 로그를 내보내며, 5분 이내의 자동 `FailureDigest` 보고서를 포함합니다.
- `UVoiceCaptureComponent`(push-to-talk 오디오 캡처) 및 `UGoogleSpeechService`(Google STT/TTS 게이트웨이)와 함께 작동하도록 설계되었습니다.

> 모듈: LatteLibrary / Character