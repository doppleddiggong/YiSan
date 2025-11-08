# Project YiSan (이산) - 개발 문서

![Game Title](https://github.com/doppleddiggong/YiSan/blob/main/Documents/Reference/Title.png?raw=true)

## 프로젝트 소개

Project YiSan은 언리얼 엔진 5.6 기반으로 개발 중인 3인칭 액션 어드벤처 게임입니다. 이 문서는 프로젝트의 전체 아키텍처, 핵심 시스템, API 및 개발 가이드라인을 제공하여 개발자의 이해를 돕고 생산성을 높이는 것을 목표로 합니다.

---

## 문서 탐색 가이드

*   **Modules (모듈)**: `전투 시스템`, `AI 시스템` 등 기능별로 그룹화된 코드 구조를 탐색할 때 사용합니다. **프로젝트의 전체 설계를 파악하는 데 가장 좋은 시작점입니다.**
*   **Classes (클래스)**: 전체 클래스 목록과 상속 구조를 알파벳순으로 확인할 수 있습니다.
*   **Files (파일)**: 모든 소스 파일과 문서 파일 목록을 보여줍니다.

---

## 핵심 시스템 (Core Systems)

아래 목록은 프로젝트의 주요 시스템과 핵심 클래스를 나타냅니다. 링크를 클릭하여 상세한 클래스 문서를 확인할 수 있습니다.

*   **코어 라이브러리 (Core Libraries)**
    *   \ref CoffeeLibrary : 공용 유틸리티 및 시스템 공통 로직 제공
    *   \ref LatteLibrary : 이동, 전투 등 게임플레이 핵심 로직 제공

*   **캐릭터 및 전투 시스템 (Character & Combat)**
    *   \ref APlayerActor : 플레이어 캐릭터의 기본 액터
    *   \ref UStatSystem : 캐릭터의 체력, 스태미나 등 스탯 관리 시스템
    *   \ref UKnockbackSystem : 피격 시 넉백 및 경직 처리
    *   \ref UHitStopSystem : 타격감 강화를 위한 히트 스톱 효과

*   **AI 시스템 (Artificial Intelligence)**
    *   \ref AYiSanAIController : NPC의 행동을 제어하는 메인 AI 컨트롤러
    *   \ref UCrowdManager : MassAI를 활용한 군중 제어 시스템

*   **상호작용 시스템 (Interaction Systems)**
    *   \ref URidingSystem : 승마 시스템
    *   \ref UVoiceInteractionSystem : 음성 인식(STT/GPT) 기반 상호작용

---

## 빠른 시작

개발 환경 설정 및 빌드 방법에 대한 자세한 내용은 [UE 프로젝트 설정 가이드](https://github.com/doppleddiggong/YiSan/blob/main/Tools/Setup_Guide.md)를 참조하십시오.