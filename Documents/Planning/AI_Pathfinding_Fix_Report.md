# AI 경로 탐색 문제 해결 리포트

## 1. 초기 문제 증상

```
✓ 첫 번째 목적지(신풍루): 이동 성공
✗ 두 번째 이후 목적지: "경로 없음" 에러 발생
✓ NavMesh는 Show Navigation으로 확인 시 존재함
? Z축이 반영되지 않는 것처럼 보임
```

### 환경
- **엔진**: Unreal Engine 5.6
- **맵 시스템**: World Partition
- **AI 시스템**: AAIController + NavigationSystem
- **군중 시스템**: Mass Crowd

---

## 2. 문제 분석 과정

### 2-1. 첫 번째 발견: 매 프레임 MoveTo 재호출

#### 문제 코드
**위치**: `UTourStateSystem::Tick_TourMove()` (Line 171-176)

```cpp
void UTourStateSystem::Tick_TourMove(float DeltaTime)
{
    // 매 Tick마다 MoveTo 호출! ← 문제!
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalActor(OwnerDasan->GetCurTargetBuilding());
    OwnerDasan->DasanAicontrol->MoveTo(MoveRequest);

    // 도착 체크
    if (OwnerDasan->IsNearTargetBuilding()) {
        CurState = ETourState::TourExplain;
    }
}
```

#### 왜 첫 번째는 성공했나?
1. `StartTour()`에서 **한 번만** MoveTo 호출
2. 경로를 찾고 이동을 완료
3. 두 번째부터는 `Tick_TourMove`가 매 프레임 MoveTo를 재호출
4. 이미 진행 중인 경로를 계속 중단하고 재계산 → 실패

#### 조치
Tick에서 MoveTo 호출 제거, Enter 함수에서만 호출하도록 변경

---

### 2-2. 두 번째 발견: SetProjectGoalLocation의 한계

#### 시도
```cpp
MoveRequest.SetGoalActor(CurTargetBuilding);
MoveRequest.SetProjectGoalLocation(true); // NavMesh로 자동 투영 시도
```

#### 결과
여전히 실패 - `SetProjectGoalLocation`이 작동하지 않음

---

### 2-3. 결정적 발견: Z축 높이 차이 문제

#### 로그 분석
```
LogCoffee: Warning: [TourState] MoveTo 시작
  From: (11884.9, 11083.1, 101.8)
  To:   (3328.2, -179.2, 377.4)
  Distance: 14146.8

LogCoffee: Warning: [TourState] 시작 위치 NavMesh: OK
LogCoffee: Warning: [TourState] 목표 위치 NavMesh: OK (투영 거리: 375.0) ← 주목!
LogCoffee: Warning: [TourState] NavMesh 투영 위치: (3328.2, -179.2, 2.4) ← Z축 차이!
LogCoffee: Warning: [TourState] 경로 찾기 성공! 경로 포인트: 12개
LogCoffee: Warning: [TourState] MoveTo 결과: 0 (RequestSuccessful)
LogCoffee: Warning: ADasanActor::OnMoveCompleted(221): AI MoveTo 실패 - 경로 없음 ← 모순!
```

#### 핵심 발견
| 항목 | 값 |
|------|-----|
| 건물 실제 위치 Z | 377.4m |
| NavMesh 위치 Z | 2.4m |
| **높이 차이** | **375m!** |

**분석**:
- `FindPathToLocationSynchronously`로 NavMesh 위치(2.4)를 사용하면 **경로 찾기 성공**
- 하지만 `SetGoalActor`는 건물의 실제 위치(377.4)를 사용
- `SetProjectGoalLocation(true)`가 **작동하지 않음!**

---

## 3. 최종 해결 방법

### SetGoalActor → SetGoalLocation 변경

#### ❌ 이전 방식 (실패)
```cpp
FAIMoveRequest MoveRequest;
MoveRequest.SetGoalActor(CurTargetBuilding);  // Actor 중심점 사용
MoveRequest.SetProjectGoalLocation(true);     // 작동 안함!
DasanAicontrol->MoveTo(MoveRequest);
```

#### ✅ 새로운 방식 (성공)
```cpp
FVector TargetPos = CurTargetBuilding->GetActorLocation();
UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

FNavLocation TargetNavLoc;
NavSys->ProjectPointToNavigation(
    TargetPos,
    TargetNavLoc,
    FVector(5000, 5000, 5000)  // 수직/수평 5000 유닛 범위에서 투영
);

FAIMoveRequest MoveRequest;
MoveRequest.SetGoalLocation(TargetNavLoc.Location);  // NavMesh 위치 직접 사용!
MoveRequest.SetAcceptanceRadius(250.0f);
MoveRequest.SetUsePathfinding(true);
DasanAicontrol->MoveTo(MoveRequest);
```

---

## 4. 수정한 위치들

### 4-1. UTourStateSystem.cpp

#### Enter_TourMove() (Line 113-162)
```cpp
void UTourStateSystem::Enter_TourMove()
{
    PRINTLOG(TEXT("[TourState] Enter TourMove"));
    WaitTimer = 0.0f;

    if (OwnerDasan && OwnerDasan->DasanAicontrol && OwnerDasan->GetCurTargetBuilding())
    {
        FVector TargetPos = OwnerDasan->GetCurTargetBuilding()->GetActorLocation();
        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

        if (NavSys)
        {
            FNavLocation TargetNavLoc;
            if (NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000)))
            {
                FAIMoveRequest MoveRequest;
                MoveRequest.SetGoalLocation(TargetNavLoc.Location);
                MoveRequest.SetAcceptanceRadius(250.0f);
                MoveRequest.SetUsePathfinding(true);

                FPathFollowingRequestResult Result = OwnerDasan->DasanAicontrol->MoveTo(MoveRequest);
                PRINTLOG(TEXT("[TourState] MoveTo 결과: %d (NavMesh 위치 사용)"), (int32)Result.Code);
            }
        }
    }
}
```

#### Tick_TourWait() (Line 281-299)
```cpp
void UTourStateSystem::Tick_TourWait(float DeltaTime)
{
    // ... (타이머 로직)

    if (IsAllPlayersNearby())
    {
        CurState = ETourState::TourMove;

        // NavMesh 위치로 이동
        if (OwnerDasan->DasanAicontrol && OwnerDasan->GetCurTargetBuilding())
        {
            FVector TargetPos = OwnerDasan->GetCurTargetBuilding()->GetActorLocation();
            UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

            if (NavSys)
            {
                FNavLocation TargetNavLoc;
                if (NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000)))
                {
                    FAIMoveRequest MoveRequest;
                    MoveRequest.SetGoalLocation(TargetNavLoc.Location);
                    MoveRequest.SetAcceptanceRadius(250.0f);
                    MoveRequest.SetUsePathfinding(true);
                    OwnerDasan->DasanAicontrol->MoveTo(MoveRequest);
                }
            }
        }
    }
}
```

### 4-2. ADasanActor.cpp

수정된 함수들:
- `StartTour()` (Line 317-356)
- `NextQuest()` (Line 535-554)
- `TransitionToState()` (Line 452-476)
- `OnMoveCompleted()` 재시도 로직 (Line 188-224)

모두 동일한 패턴으로 수정:
1. NavigationSystem 획득
2. ProjectPointToNavigation으로 NavMesh 위치 계산
3. SetGoalLocation 사용

---

## 5. 왜 SetProjectGoalLocation이 작동하지 않았나?

### 추측

#### 가능성 1: 제한된 투영 범위
```cpp
// SetProjectGoalLocation 내부 구현 (추정)
bool FAIMoveRequest::ProjectGoalLocation()
{
    // 수직 범위가 제한적 (예: 500 유닛)
    NavSys->ProjectPointToNavigation(GoalLocation, ProjectedLocation, FVector(500, 500, 500));
}
```
- 375m 높이 차이는 기본 범위를 초과
- 투영 실패 → 원래 위치 사용 → 경로 탐색 실패

#### 가능성 2: Actor Bounds 기반 투영
- Actor의 Bounds 중심점을 기준으로 투영
- 건물이 너무 크거나 복잡한 구조여서 문제 발생

### 결론
수동으로 `NavigationSystem::ProjectPointToNavigation`을 호출하고 `SetGoalLocation`을 사용하는 것이 **확실한 방법**!

---

## 6. 추가 발견: 퀘스트 진행 문제

### 보너스 문제
여민각에서 별주로 진행되지 않는 이유

#### 원인
- 다산이 건물에 도착해도 `OnContactBuilding` 이벤트가 발생하지 않음
- `ABuilding::OnOverlapBegin`이 모든 액터를 처리하지만 **필터링이 없었음**

#### 해결

**ADasanActor.cpp** - 생성자에 태그 추가:
```cpp
ADasanActor::ADasanActor()
{
    // ...
    Tags.Add(TEXT("Dasan")); // 퀘스트 인식용 태그
}
```

**ABuilding.cpp** - Overlap 필터링:
```cpp
void ABuilding::OnOverlapBegin(...)
{
    if (OtherActor && (OtherActor != this))
    {
        // 플레이어나 다산만 퀘스트 진행
        bool bIsPlayer = OtherActor->IsA<APlayerActor>();
        bool bIsDasan = OtherActor->ActorHasTag(TEXT("Dasan"));

        if (bIsPlayer || bIsDasan)
        {
            PRINTLOG(TEXT("[Building] %s entered %s (BuildingType: %s)"),
                *OtherActor->GetName(),
                *this->GetName(),
                *ENUM_TO_NAME(EBuildingType, BuildingType));

            BroadcastManager->SendContactBuilding(BuildingType);
        }
    }
}
```

**UQuestManager.cpp** - 디버그 로그 추가:
```cpp
void UQuestManager::OnContactBuilding(EBuildingType InType)
{
    if (InType == CurTarget)
    {
        PRINTLOG(TEXT("[QuestManager] Quest Completed: %s (Index: %d)"),
            *TargetName, CurQuestIndex);

        CurQuestIndex++;

        if (QuestList.IsValidIndex(CurQuestIndex))
        {
            CurTarget = QuestList[CurQuestIndex];
            PRINTLOG(TEXT("[QuestManager] 다음 목표: %s"),
                *ENUM_TO_NAME(EBuildingType, CurTarget));
        }
        else
        {
            PRINTLOG(TEXT("[QuestManager] 모든 퀘스트 완료!"));
            CurTarget = EBuildingType::None;
        }
    }
}
```

---

## 7. 요약

### 문제
`SetGoalActor`가 건물의 실제 높이(377m)를 사용하는데, NavMesh는 지면(2m)에만 있어서 경로 탐색 실패

### 해결
1. NavigationSystem으로 목표 위치를 NavMesh에 **수동 투영**
2. `SetGoalLocation`으로 직접 NavMesh 위치 사용
3. Tick에서 반복 MoveTo 호출 제거

### 교훈
**World Partition + 건물 높이 차이가 큰 환경**에서는:
- ❌ `SetGoalActor` + `SetProjectGoalLocation(true)`
- ✅ `ProjectPointToNavigation` + `SetGoalLocation`

### 수정된 파일 목록
1. `UTourStateSystem.cpp` - Enter/Tick 함수에서 NavMesh 투영 사용
2. `ADasanActor.cpp` - 모든 MoveTo 호출 지점에 NavMesh 투영 적용
3. `ABuilding.cpp` - 플레이어/다산 필터링 추가
4. `UQuestManager.cpp` - 퀘스트 진행 로그 추가

---

## 8. 참고 코드 패턴

### 표준 NavMesh 기반 이동 패턴

```cpp
bool MoveToNavMeshLocation(AAIController* AIController, const FVector& TargetLocation, float AcceptanceRadius = 250.0f)
{
    if (!AIController)
        return false;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(AIController->GetWorld());
    if (!NavSys)
        return false;

    // NavMesh로 투영 (수직/수평 5000 유닛 범위)
    FNavLocation NavLocation;
    if (!NavSys->ProjectPointToNavigation(TargetLocation, NavLocation, FVector(5000, 5000, 5000)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to project location to NavMesh"));
        return false;
    }

    // 이동 요청 생성
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(NavLocation.Location);
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
    MoveRequest.SetUsePathfinding(true);

    // 이동 실행
    FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest);
    return Result.Code == EPathFollowingRequestResult::RequestSuccessful;
}
```

---

*작성일: 2025년 1월*
*프로젝트: YiSan*
*엔진: Unreal Engine 5.6*
