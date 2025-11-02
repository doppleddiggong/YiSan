# Project Scope Summary — Oct 2025
# 프로젝트 전체 범위 요약 — 2025년 10월

## Overview / 개요
- Timeframe: 2025-10-01 ~ 2025-10-31 (KST)  
  기간: 2025-10-01 ~ 2025-10-31 (KST)
- Primary contributor identity: dopple (aka Ju100) with supporting branches NEW_NEO_JM, kbm for art/UI integration.  
  주요 기여자: dopple(Ju100) 단일 계정으로 활동, NEW_NEO_JM·kbm 브랜치가 아트·UI 통합을 보조했습니다.【F:Documents/DevLog/2025-10-31.md†L41-L79】【F:Documents/DevLog/2025-10-31-weekend-overview.md†L5-L33】
- Aggregate activity: 56 commits across 3 active days captured in monthly log, with +9,449 / -4,828 LOC (up to Oct-03) plus late-month surges for voice/UI systems.【F:Documents/DevLog/Monthly/2025-10.md†L5-L58】【F:Documents/DevLog/2025-10-26.md†L69-L83】

## Branch-Level Streams / 브랜치 단위 주요 흐름
1. **Documentation & Tooling (main/docs)**  
   - Established Doxygen documentation, DevLog automation, and CI workflows at the start of the month.【F:Documents/DevLog/Monthly/2025-10.md†L9-L83】  
   - Maintained ongoing DevLog updates and weekly briefings, ensuring traceability for later features.【F:Documents/DevLog/2025-10-25.md†L21-L53】【F:Documents/DevLog/2025-W43-Summary.md†L1-L61】
2. **Voice Interaction & NPC Tour (voice/dasan)**  
   - Added Google STT + GPT narration support, culminating in Dasan tour state machines and broadcast integration during Oct 26 sprint.【F:Documents/DevLog/Monthly/2025-10.md†L25-L41】【F:Documents/DevLog/2025-10-26.md†L5-L83】  
   - Implemented summon voice commands, resampling, and LocalPlayerSubsystem-based dialog services to stabilize runtime use.【F:Documents/DevLog/2025-10-26.md†L5-L76】
3. **Multiplayer Loading & Network (loading/net)**  
   - Iteratively improved loading screens, readiness gating, and matchmaking flows, including rollback and real-time handling adjustments.【F:Documents/DevLog/2025-10-25.md†L5-L33】【F:Documents/DevLog/2025-10-30.md†L5-L33】  
   - Introduced IP join options, timer instrumentation, and PlayerState synchronization on Oct 31 to support QA across branches.【F:Documents/DevLog/2025-10-31.md†L5-L79】
4. **UI/UX & Asset Integration (NEW_NEO_JM, kbm)**  
   - Processed mega/small popup redesigns, tour UI, and cinematic assets through dedicated feature branches before merging into work.【F:Documents/DevLog/2025-10-26.md†L44-L83】【F:Documents/DevLog/2025-10-31.md†L41-L79】  
   - Merged Metahuman clothing updates and intro/outro sequences while coordinating to avoid overwriting gameplay-layer changes.【F:Documents/DevLog/2025-10-31.md†L41-L67】【F:Documents/DevLog/2025-10-31-weekend-overview.md†L23-L43】

## Cross-Branch Outcomes / 교차 브랜치 성과
- Unified chat/nameplate UX leveraging PlayerIndex data ensures consistent identity presentation across branches.【F:Documents/DevLog/2025-10-31.md†L5-L79】
- Voice pipeline refactor plus asset imports enable narrated tours with synchronized UI overlays, bridging voice and UI teams.【F:Documents/DevLog/2025-10-26.md†L5-L83】
- Documentation cadence (daily/weekly/monthly) provides historical baseline for upcoming November milestones.【F:Documents/DevLog/2025-W43-Summary.md†L1-L96】【F:Documents/DevLog/2025-10-31-weekend-overview.md†L35-L46】

## Outstanding Risks / 잔여 리스크
- Merge coordination across NEW_NEO_JM and kbm may override latest loading/chat fixes; enforce regression passes post-merge.【F:Documents/DevLog/2025-10-31.md†L41-L79】
- Asset growth from tour audio and clothing requires build size monitoring and packaging policy updates.【F:Documents/DevLog/2025-10-26.md†L82-L114】
- Voice summon/testing scenarios remain pending, necessitating latency QA and ASK feature completion.【F:Documents/DevLog/2025-10-26.md†L84-L114】
