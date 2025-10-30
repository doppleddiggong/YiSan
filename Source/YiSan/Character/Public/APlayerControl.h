// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file APlayerControl.h
 * @brief APlayerControl 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/PlayerController.h"
#include "APlayerControl.generated.h"

struct FInputActionValue;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple))
class YISAN_API APlayerControl : public APlayerController
{
	GENERATED_BODY()
	
public:
	APlayerControl();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	// --- Input Assets ---
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<class UInputMappingContext> IMC_Default;	

	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Move;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Look;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_AltitudeUp;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_AltitudeDown;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Jump;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Landing;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Chat;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_ChatScrollUp;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_ChatScrollDown;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Record;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_ShowDetail;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_ShowMouse;

	// --- Handlers ---
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnAltitudeUp(const FInputActionValue& Value);
	void OnAltitudeDown(const FInputActionValue& Value);
	void OnAltitudeReleased(const FInputActionValue& Value);
	
	void OnJump(const FInputActionValue& Value);
	void OnLanding(const FInputActionValue& Value);

	void OnChatEnter(const FInputActionValue& Value);
	void OnChatScrollUp(const FInputActionValue& Value);
	void OnChatScrollDown(const FInputActionValue& Value);
	void OnRecordPressed(const FInputActionValue& Value);
	void OnRecordReleased(const FInputActionValue& Value);

	void OnShowDetail(const FInputActionValue& Value);
	void OnShowMouse(const FInputActionValue& Value);
	void OnHideMouse(const FInputActionValue& Value);

public:
	void OnPawnReady(class APawn& InPawn);
	
private:
	UFUNCTION()
	void OnPlayerControlState(bool bState, class UUserWidget* FocusWidget);

	void CompleteLoading();
	bool IsReadyToFinish() const;

	
#pragma region LOADING
	//----------------로딩 관련-------------------
	// 서버에서 호출: 맵 전환 시작
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void ServerStartMapTravel(const FString& MapPath);

	// 클라이언트가 서버에게 맵 전환 요청 (Server RPC)
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Travel")
	void Server_RequestMapTravel(const FString& MapPath);

	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowLoadingTransition();

	UFUNCTION(Client, Reliable)
	void ClientRPC_HideLoadingTransition();
	
	void HandleLoadingComplete();
#pragma endregion LOADING


#pragma region RECORDING
	// Recording RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_NotifyRecordingStart();

	UFUNCTION(Server, Reliable)
	void ServerRPC_NotifyRecordingEnd();
#pragma endregion RECORDING

	
#pragma region ANSWER
	// Answer RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_TryStartAnswer(const FString& PlayerName);

	UFUNCTION(Server, Reliable)
	void ServerRPC_AnswerReply();

	UFUNCTION(Server, Reliable)
	void ServerRPC_FinishAnswer();
#pragma endregion ANSWER

	
	// Toast
	UFUNCTION(Server, Reliable)
	void ServerRPC_ShowToastMessage(const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowToastMessage(const FString& Message);

	
	// Quest
	UFUNCTION(Client, Reliable)
	void ClientRPC_UpdateQuestTarget(const EBuildingType BuildingType);

	// NickName
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerNickname(const FString& Nickname);

private:
    class IControllable* GetControllable() const;

	bool bAwaitFinish = false;
	bool bPawnReady = false;
	

	// 마지막 토스트 전송 시간                                                                                                                                          
	float LastToastTime = 0.f;
	// 쿨다운 (초)                                                                                                                                                      
	const float ToastCooldown = 2.0f;
	bool CanSendToast() const                                                                                                                                           
	{
		// 현재 시간 확인                                                                                                                                               
		float CurrentTime = GetWorld()->GetTimeSeconds();

		// 쿨다운 체크                                                                                                                                                  
		if (CurrentTime - LastToastTime < ToastCooldown)
		{
			return false; // 너무 빠른 재호출                                                                                                                           
		}

		return true;
	}
};
