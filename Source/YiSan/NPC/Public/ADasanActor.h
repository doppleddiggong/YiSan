#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "EDasanState.h"
#include "Navigation/PathFollowingComponent.h"
#include "ADasanActor.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTryStartAnswer, const FString&);
DECLARE_MULTICAST_DELEGATE(FOnFinishAnswer);

UCLASS()
class YISAN_API ADasanActor : public ACharacter
{
	GENERATED_BODY()

public:
	ADasanActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// RepNotify 함수
	UFUNCTION()
	void OnRep_DasanState();

	// 서버 RPC - 상태 변경
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetDasanState(EDasanState InState);
	
	// AI MoveTo 완료 콜백
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	// 현재 메인 상태 (Tour/Explain/Answer)
	UPROPERTY(ReplicatedUsing=OnRep_DasanState, BlueprintReadOnly, Category="State")
	EDasanState DasanState;
	FORCEINLINE EDasanState GetDasanState() const { return DasanState; }
	
public:
	// 유틸리티 함수
	void StartTour();
	void NextQuest();

	bool IsNearTargetBuilding();
	
	// 현재 목표 건물 찾기
	class ABuilding* FindCurTargetBuilding() const;
	class ABuilding* GetCurTargetBuilding() { return CurTargetBuilding;}
	FORCEINLINE void UpdateTargetBuilding(class ABuilding* InBuilding)	{ CurTargetBuilding = InBuilding; }
	
	// 상태 전환 메서드
	void TransitionToState(EDasanState InMainState);

	float GetTargetBuildingDistnace();

	void TryStartAnswer(const FString& PlayerName);
	void FinishAnswer();

	FOnTryStartAnswer OnTryStartAnswer;
	FOnFinishAnswer OnFinishAnswer;
	
	FPathFollowingRequestResult AIMoveToLoc(
		const FVector& InGoalLocation,
		const float InAcceptanceRadius,
		const bool bPathfinding ) const;

	FPathFollowingRequestResult AIMoveToActor(
		const AActor* Actor,
		const float InAcceptanceRadius,
		const bool bPathfinding ) const;

public:
	// 상태 시스템 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	TObjectPtr<class UTourStateSystem> TourStateSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	TObjectPtr<class UAnswerStateSystem> AnswerStateSystem;

	// 다산 상태 표시 위젯 컴포넌트 (머리 위에 표시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<class UWidgetComponent> DasanWidgetComp;

	// 캐싱된 다산 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<class UDasanWidget> DasanWidget;

	// 위젯 상태 업데이트 함수
	void UpdateWidgetState();

	// 플레이어 폰을 가져오는 헬퍼 함수
	class APawn* GetPlayerPawn() const;
	
	// 플레이어와의 거리를 계산하는 함수
	float GetPlayerDistance(class APawn* PlayerPawn) const;
	
	// ai control
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class AAIController> DasanAicontrol;
	 
	// 플레이어 최대 거리
	float playerMaxDis;
	
	// tour wait 상태에서 player 체크용
	float waitChackTimer;
	
	// 웨이포인트 거리
	float wayPointDis;

public:	
	UPROPERTY()
	TObjectPtr<class UQuestManager> QuestManager;

	UPROPERTY()
	TObjectPtr<class ABuilding> CurTargetBuilding;

	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;

private:
	// 음성 명령 핸들러
	UFUNCTION()
	void OnExecVoiceCommand(EVoiceCommandType InType, AActor* Requester);
};