# Weekend Work Overview — 2025-10-25 ~ 2025-10-26
# 주말 작업 개요 — 2025-10-25 ~ 2025-10-26

## Summary / 요약
- Focused on stabilizing the lobby/voice pipeline while extending Dasan NPC tour narration flows.
- 로비·음성 파이프라인을 안정화하는 동시에 다산 NPC 투어 내레이션 흐름을 확장했습니다.
- Ju100 and dopple operated as a single contributor identity, coordinating rollbacks, documentation, and voice-stack upgrades.
- Ju100과 dopple은 동일 인물로 간주되며, 롤백·문서화·음성 스택 업그레이드를 일괄 조율했습니다.【F:Documents/DevLog/2025-10-25.md†L5-L23】

## Major Buckets / 대분류
1. **Voice & NPC Tour Integration / 음성·NPC 투어 통합**  
   - Expanded Dasan NPC tour states, broadcast linkage, and dialog UI to handle narrated tours end-to-end.【F:Documents/DevLog/2025-10-26.md†L5-L63】  
   - Added summon intent, resampling, and audio asset ingestion to unify voice commands and narration quality.【F:Documents/DevLog/2025-10-26.md†L5-L83】
2. **Lobby & Loading Stability / 로비·로딩 안정화**  
   - Restricted GPT context ticking to locally controlled pawns and rolled back experimental lobby UI to maintain matchmaking reliability.【F:Documents/DevLog/2025-10-25.md†L5-L33】  
   - Introduced global loading UI managers and asynchronous status widgets for consistent UX across server/client roles.【F:Documents/DevLog/2025-10-26.md†L44-L75】
3. **UI/UX Polish & Asset Management / UI·UX 다듬기 및 자산 관리**  
   - Delivered popup state/UI refinements, state indicators, and channel-specific chat improvements.【F:Documents/DevLog/2025-10-26.md†L44-L76】  
   - Imported building audio, textures, and documentation updates to keep reference material aligned.【F:Documents/DevLog/2025-10-26.md†L69-L83】
4. **Documentation & Process Hygiene / 문서화·프로세스 정비**  
   - Updated previous-day devlogs and enumerated follow-up TODOs for lobby redesign and QA scenarios.【F:Documents/DevLog/2025-10-25.md†L21-L53】  
   - Captured outstanding ASK/voice TODOs and next steps for automation and QA coverage.【F:Documents/DevLog/2025-10-26.md†L84-L114】

## Quantitative Snapshot / 정량 지표
- **Commits / 커밋**: 2 (10-25) + 13 (10-26) = 15  
- **Changes / 변경량**: +150 / -163 (10-25), +3,340 / -2,086 (10-26)  
- **Files / 파일 수**: 10 + 379 = 389  
- Source references show that Saturday was light (rollback & documentation) while Sunday involved large-scale system and asset pushes.【F:Documents/DevLog/2025-10-25.md†L34-L53】【F:Documents/DevLog/2025-10-26.md†L69-L117】

## Carry-over Risks & Next Steps / 잔여 리스크 및 다음 단계
- Plan a phased lobby redesign after the rollback to avoid regression while modernizing the flow.【F:Documents/DevLog/2025-10-25.md†L37-L53】
- Validate resampling and summon voice commands under multiplayer latency conditions.【F:Documents/DevLog/2025-10-26.md†L84-L114】
- Monitor package size growth from mass asset imports and establish QA checkpoints for ASK completion.【F:Documents/DevLog/2025-10-26.md†L82-L114】
