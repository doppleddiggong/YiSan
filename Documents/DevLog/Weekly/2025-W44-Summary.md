# 주간 개발 요약 (2025-10-27 ~ 2025-11-02)

## 주간 개요 / Weekly Overview
- **로딩·세션 전환 체계 완성**: GameInstanceSubsystem, LocalPlayerSubsystem, LoadingTransition 위젯으로 전환 로딩이 중앙화되어 모든 플레이어에게 동기화된 UX를 제공하게 되었습니다.【F:Documents/DevLog/Daily/2025-10-30.md†L15-L86】【F:Documents/DevLog/Daily/2025-10-31.md†L17-L74】
- **플레이어 목록·채팅·투어 UI 재정비**: PlayerIndex 기반 목록, Mega/Small 팝업, 투어 다이얼로그 개선으로 로비·투어 UI 일관성이 향상되었습니다.【F:Documents/DevLog/Daily/2025-10-31.md†L17-L80】【F:Documents/DevLog/Daily/2025-11-02.md†L15-L169】
- **음성 명령과 사운드 파이프라인 확장**: 16kHz 리샘플링 이후 음성 명령 세트, 가이드 UI, GameState 기반 사운드 재생이 연결되어 몰입형 투어 경험을 제공할 준비를 마쳤습니다.【F:Documents/DevLog/Daily/2025-10-27.md†L3-L23】【F:Documents/DevLog/Daily/2025-11-02.md†L15-L136】

## 핵심 성과 / Key Achievements
1. **멀티플레이 로딩 전환 중앙화** — `UYiSanLoading`과 `ULoadingTransitionManager`를 통해 로딩 표시·완료 조건이 통합되었고, 최소 표시 시간·동기화 RPC가 마련되었습니다.【F:Documents/DevLog/Daily/2025-10-30.md†L15-L86】【F:Documents/DevLog/Daily/2025-10-31.md†L17-L60】
2. **퀘스트·투어 상태 머신 안정화** — QuestManager Actor 전환, Tour 상태 전환 버그 수정, BroadcastManager 기반 설명 흐름으로 투어/퀘스트 동기화가 강화되었습니다.【F:Documents/DevLog/Daily/2025-10-30.md†L21-L99】【F:Documents/DevLog/Daily/2025-11-02.md†L17-L87】
3. **플레이어 UI/네트워크 가시성 향상** — PlayerIndex·아이콘 기반 목록, 채팅 컬러, MegaPopup 이미지 교체 등 UI 업데이트가 진행되어 역할 구분과 피드백 속도가 향상되었습니다.【F:Documents/DevLog/Daily/2025-10-31.md†L19-L80】【F:Documents/DevLog/Daily/2025-11-02.md†L25-L169】
4. **음성 명령 UX 고도화** — 음성 명령 가이드 패널, GameMessage 연동, 사운드 중앙화로 음성 상호작용이 UI·오디오와 함께 체계화되었습니다.【F:Documents/DevLog/Daily/2025-11-02.md†L18-L136】

## 리스크 및 이슈 / Risks & Issues
- **대규모 캐릭터 리소스 추가**: Nov.2에 메타휴먼 파츠 284개가 추가되어 패키지 용량과 빌드 시간이 증가할 가능성이 큽니다.【F:Documents/DevLog/Daily/2025-11-02.md†L37-L49】【F:Documents/DevLog/Daily/2025-11-02.md†L75-L107】
- **Collision/Debug 반복 롤백**: 10/29경 충돌 설정 변경과 롤백이 반복되었으며, Mass 관련 버그가 완전히 해결되지 않았을 수 있습니다.【F:Documents/DevLog/Daily/2025-10-29.md†L23-L44】
- **로딩 파이프라인 다중 구현 잔존**: 새 전환 시스템과 Legacy 로그 비교 흐름이 병존하므로, 중복 경로 정리와 QA가 필요합니다.【F:Documents/DevLog/Daily/2025-10-30.md†L15-L58】

## 다음 주 우선순위 / Next Week Priorities
1. **패키징·빌드 성능 점검** — 대량 캐릭터 리소스와 새로운 로딩 시스템이 빌드 시간, 메모리, 패키징 오류에 미치는 영향을 계측합니다.【F:Documents/DevLog/Daily/2025-11-02.md†L32-L49】【F:Documents/DevLog/Daily/2025-11-02.md†L75-L176】
2. **충돌·Mass 시뮬레이션 검증** — 롤백된 충돌 변경과 Mass BUG FIX 커밋을 재검증해 잔여 충돌/AI 이슈를 제거합니다.【F:Documents/DevLog/Daily/2025-10-29.md†L23-L44】
3. **음성 명령 가이드 플로우 QA** — 가이드 UI, GameMessage, 사운드 매니저를 포함해 음성 명령 전체 플로우를 멀티 세션 기준으로 테스트합니다.【F:Documents/DevLog/Daily/2025-11-02.md†L18-L136】

## 역할별 리뷰 / Role-based Review
- **클라이언트팀장 관점**: 로딩 전환과 플레이어 UI가 일관되게 정리되어 사용자 여정이 개선되었지만, 대량 리소스 탑재에 따른 초기 로딩/프레임 영향을 지속 관찰해야 합니다.【F:Documents/DevLog/Daily/2025-10-30.md†L15-L86】【F:Documents/DevLog/Daily/2025-11-02.md†L37-L136】
- **네트워크 리드 관점**: PlayerControl RPC 중앙화, GameState 사운드 재생 덕분에 네트워크 경로가 단순화되었으나 Stage 환경에서의 재현 테스트와 패키징 오류 점검이 필요합니다.【F:Documents/DevLog/Daily/2025-10-28.md†L17-L124】【F:Documents/DevLog/Daily/2025-11-02.md†L18-L143】
- **콘텐츠/디자인 관점**: Mega/Small 팝업, 투어 다이얼로그, 음성 가이드가 확보되어 스토리텔링 자산 활용도가 높아졌고, 남은 시네마틱·BGM 연동을 마무리해야 합니다.【F:Documents/DevLog/Daily/2025-10-31.md†L28-L43】【F:Documents/DevLog/Daily/2025-11-02.md†L31-L58】

## 구성원 역량 평가 / Individual Competency Review
- **dopple**: 로딩/퀘스트/음성 명령/사운드까지 네트워크-UI-오디오 전반을 연결하며 시스템 통합 역량을 입증했습니다. 후속 QA 계획과 패키징 대응 전략을 명확히 해야 합니다.【F:Documents/DevLog/Daily/2025-10-30.md†L15-L86】【F:Documents/DevLog/Daily/2025-11-02.md†L17-L169】
- **Ju100**: 로딩 성능 비교 로그와 DevLog 백필을 통해 개선 효과를 정량화하고 팀 기록을 보강했습니다. 향후 대량 리소스 증분에 대한 문서화 및 모니터링을 이어가야 합니다.【F:Documents/DevLog/Daily/2025-10-30.md†L15-L58】【F:Documents/DevLog/Daily/2025-10-27.md†L13-L23】
- **kbm**: 캐릭터 파츠, 이름표, UI 자산을 폭넓게 보강해 시각적 완성도를 높였습니다. 대규모 uasset 병합 시 충돌·빌드 영향 분석을 강화할 필요가 있습니다.【F:Documents/DevLog/Daily/2025-10-31.md†L27-L43】【F:Documents/DevLog/Daily/2025-11-02.md†L37-L58】

## 메트릭 / Metrics
- **주간 변경량**: +15,044 / -8,294 (대규모 UI·리소스 업데이트 포함).【F:Documents/DevLog/Daily/2025-10-27.md†L3-L43】【F:Documents/DevLog/Daily/2025-11-02.md†L4-L176】
- **활동 일수**: 7/7일 — 매일 커밋 및 문서 업데이트 지속.【F:Documents/DevLog/Daily/2025-10-27.md†L3-L42】【F:Documents/DevLog/Daily/2025-11-02.md†L4-L176】
- **주요 테마**: 멀티플레이 로딩 UX 통합, 플레이어 목록/채팅 개선, 음성 명령 & 사운드 파이프라인 확장.
