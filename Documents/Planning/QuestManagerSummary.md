# QuestManager 작업 요약

## 주요 코드 변경
- `Source/YiSan/Quest/Public/UQuestManager.h`: `UQuestManager` 서브시스템 선언 및 `FOnUpdateQuest` 델리게이트 추가.
- `Source/YiSan/Quest/Private/UQuestManager.cpp`: 기본 퀘스트 목록 초기화, `UBroadcastManger::OnContactBuilding` 연결, 목표 일치 여부에 따른 진행/토스트 처리 구현.
- `Source/YiSan/UI/Public/UStateWidget.h`: 퀘스트 텍스트 바인딩을 위한 `QuestTargetText` 추가와 `OnQuestUpdated` 핸들러 선언, 퀘스트 상태 캐시 멤버 확장.
- `Source/YiSan/UI/Private/UStateWidget.cpp`: 퀘스트 매니저 구독/재연결 로직과 HUD 텍스트 갱신, 초기 상태 정리.
- `Source/YiSan/YiSan.Build.cs`: `Quest/Public` 경로를 `PublicIncludePaths`에 추가하여 새 헤더가 모듈 전역에서 노출되도록 수정.

## 시스템 흐름
1. `UQuestManager`가 초기화 단계에서 `UBroadcastManger`를 의존성으로 선언하고 기본 퀘스트 리스트를 구성합니다.
2. `UBroadcastManger::SendContactBuilding` 호출 시 `HandleBuildingContact`가 트리거되어 목표 일치 여부를 판정합니다.
   - 목표와 일치하면 `PRINT_STRING`으로 완료 메시지를 출력하고 `AdvanceToNextQuest`로 다음 목표를 설정합니다.
   - 다르면 현재 목표명을 포함한 토스트 메시지를 전송합니다.
3. 목표가 갱신되면 `OnUpdateQuest` 델리게이트가 브로드캐스트되어 UI가 최신 목표를 표시합니다. 더 이상 목표가 없으면 `EBuildingType::None`을 보내 UI를 숨깁니다.

## UI 연동 요약
- `UStateWidget`은 `NativeConstruct`에서 `UQuestManager`를 조회하여 델리게이트를 구독하고, `QuestTargetText`에 목표 건물명을 출력합니다.
- 런타임 중 서브시스템이 늦게 생성되는 경우를 대비해 `NativeTick`에서 재연결을 시도하며, 목표 상태가 바뀌면 즉시 텍스트를 갱신합니다.
- 목표가 없을 때는 텍스트를 지우고 위젯을 숨겨 HUD가 깔끔하게 유지됩니다.

## 후속 점검 항목
- 실제 건물 충돌/접촉 로직에서 `UBroadcastManger::SendContactBuilding`이 호출되는지 확인하고, 없다면 해당 호출을 추가하세요.
- UI 블루프린트에 `QuestTargetText`(`TextBlock`)가 바인딩되어 있는지 점검하세요.
- `Documents/MasterData/MasterData - Building.csv`, `Source/LatteLibrary/Data/Public/FBuildingData.h` 등 기존 변경 파일은 이번 작업과 무관하므로 필요 시 별도 검토가 필요합니다.
