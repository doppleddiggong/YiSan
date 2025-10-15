당신은 언리얼 엔진 C++ 기반의 퀘스트 매니저 시스템을 설계하고 구현하는 시니어 프로그래머입니다.
다음 요구사항에 맞는 클래스를 설계하고, .h/.cpp 코드 구조, 이벤트 바인딩 흐름, 필요한 함수들을 구현해주세요.

--------------------------------------------------
🎯 목표 시스템 개요
- 환경: Unreal Engine 5 C++
- 퀘스트 매니저는 UGameInstanceSubsystem 기반으로 설계한다.
- 클래스명: UQuestManager
- 헤더/CPP 분리 구조로 작성한다.

--------------------------------------------------
📌 주요 데이터 구조
1) 퀘스트 목록:
TArray<EBuildingType> QuestList;

2) 현재 목표 건물:
EBuildingType CurTarget;

3) 퀘스트 상태 이벤트:
- 건물 도착 이벤트 수신: FOnContactBuilding OnContactBuilding;
- 다음 퀘스트 갱신 이벤트(위젯 전달용): FOnUpdateQuest;

--------------------------------------------------
📌 작동 흐름

① 특정 건물에 도착하면 이벤트가 발생한다:
FOnContactBuilding(EBuildingType ReachedType);

② 도착한 건물이 현재 목표(CurTarget)와 같다면:
- "도착완료"라는 문자열을 PRINT_STRING 또는 PRINTLOG로 출력.
- 다음 퀘스트로 넘어간다.

③ 도착한 건물이 목표와 다르다면:
- SendToastMessage("○○ 도착완료") 형태로 토스트 메시지 출력.

④ 다음 퀘스트로 전환될 때:
- QuestList에서 다음 인덱스의 건물을 CurTarget으로 설정한다.
- 다음 목표 정보를 포함한 FOnUpdateQuest 이벤트를 호출한다.
- UStateWidget 또는 HUD/UI 쪽에서 이를 수신하게 된다.

⑤ 더 이상 진행할 퀘스트가 없다면:
- CurTarget = EBuildingType::None 상태가 된다.
- 이벤트를 더 이상 발생시키지 않는다.
- UI에도 출력하지 않는다.

--------------------------------------------------
📌 초기 설정
- UQuestManager는 게임 시작 시 또는 Init 단계에서 QuestList를 세팅한다.
- CurTarget은 첫 번째 퀘스트로 자동 지정하거나 SetInitialQuest 함수에서 설정한다.

--------------------------------------------------
📌 필요한 함수 예시

class UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

private:
    TArray<EBuildingType> QuestList;
    int32 CurrentQuestIndex;
    EBuildingType CurTarget;

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    void SetQuestList(const TArray<EBuildingType>& InList);
    void StartQuest();
    void HandleBuildingContact(EBuildingType InType);
    void AdvanceToNextQuest();
    EBuildingType GetCurrentTarget() const;

    // Delegate: FOnUpdateQuest / 바인딩 선언 필요
};

--------------------------------------------------
📌 구현 조건
- .h, .cpp로 나눈 구조
- 델리게이트 바인딩과 브로드캐스트 처리 포함
- SendToastMessage, PrintString, UpdateStateWidget 호출 지점 명확히 작성
- CurTarget == None일 때의 처리 포함

--------------------------------------------------
📌 출력 형태
1) 헤더(.h)
2) CPP(.cpp)
3) 예시 Delegates 선언부
4) BeginPlay 또는 Init에서 바인딩 예시
5) UStateWidget과 연결되는 방식 (예: 브로드캐스트 함수 예시)

이 모든 내용을 C++ 코드 형태로 바로 사용할 수 있도록 작성하라.
