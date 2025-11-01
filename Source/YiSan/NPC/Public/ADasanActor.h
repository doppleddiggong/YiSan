#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "EDasanState.h"
#include "Navigation/PathFollowingComponent.h"
#include "ADasanActor.generated.h"

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
	
	// AI MoveTo 완료 콜백
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	// 현재 메인 상태 (Tour/Explain/Answer)
	UPROPERTY(ReplicatedUsing=OnRep_DasanState, BlueprintReadOnly,Category="State")
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
	
	FPathFollowingRequestResult AIMoveToLoc(
		const FVector& InGoalLocation,
		const float InAcceptanceRadius,
		const bool bPathfinding );

	FPathFollowingRequestResult AIMoveToActor(
		const AActor* Actor,
		const float InAcceptanceRadius,
		const bool bPathfinding );

	// 위젯 상태 업데이트 함수
	void UpdateWidgetState();

	// 모든 클라이언트에게 설명 다이얼로그 표시 (서버에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void ShowExplainDialog(const FString& ExplainText);
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void HideExplainDialog();

	// 다산의 채팅 메시지를 모든 클라이언트에게 전송
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendDasanChatMessage(const FString& Message);

	// 플레이어 폰을 가져오는 헬퍼 함수
	class APawn* GetPlayerPawn() const;
	
	// 플레이어와의 거리를 계산하는 함수
	float GetPlayerDistance(class APawn* PlayerPawn) const;
	

private:
	void DebugDrawPath(const FVector& GoalLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_DrawDebugPath(const TArray<FVector>& PathPoints);

	// 모든 클라이언트에게 설명 다이얼로그 표시
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_ShowExplainDialog(const FString& ExplainText);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_HideExplainDialog();

	// 다산의 채팅 메시지 전송 RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_SendDasanChatMessage(const FString& Message);

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

	// ai control
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class AAIController> DasanAicontrol;
	 
	// 플레이어 최대 거리
	float playerMaxDis;
	
	// tour wait 상태에서 player 체크용
	float waitChackTimer;
	
	// 웨이포인트 거리
	float wayPointDis;

	UPROPERTY(EditAnywhere, Category="Debug|Dasan")
	bool bEnableDebugDraw = true;

	
public:	
	UPROPERTY()
	TObjectPtr<class AQuestManagerActor> QuestManager;

	UPROPERTY()
	TObjectPtr<class ABuilding> CurTargetBuilding;

	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;

private:
	// 음성 명령 핸들러
	UFUNCTION()
	void OnExecVoiceCommand(EVoiceCommandType InType, AActor* Requester);

public:
	// ---------------------------- ABP 관련 함수 -------------------------------//

	//이동 속도 
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dasan|Animation")
	float GetGroundSpeed();

	//상태 확인 함수 
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dasan|Animation")
	bool IsExplaining(); 

	// Explain 시작 
	UFUNCTION(BlueprintCallable, Category = "Dasan|Animation")
	void StartExplainAnim();

	// Explain 종료 
	UFUNCTION(BlueprintCallable, Category = "Dasan|Animation")
	void EndExplainAnim();

	// 애님 재생중 여부
	UPROPERTY(Replicated)
	bool bIsPlayingExplainAnim;
};