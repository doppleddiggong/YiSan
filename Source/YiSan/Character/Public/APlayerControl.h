// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file APlayerControl.h
 * @brief APlayerControl 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
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
	//----------------로딩 관련-------------------
	
	// 로딩 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	// 서버에서 호출: 맵 전환 시작
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void ServerStartMapTravel(const FString& MapPath);

	// 클라이언트가 서버에게 맵 전환 요청 (Server RPC)
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Travel")
	void Server_RequestMapTravel(const FString& MapPath);
	void Server_RequestMapTravel_Implementation(const FString& MapPath);

	// 클라이언트 RPC: 로딩 UI 표시
	UFUNCTION(Client, Reliable)
	void Client_ShowLoadingScreen();
	void Client_ShowLoadingScreen_Implementation();

	// 클라이언트 RPC: 로딩 UI 숨김
	UFUNCTION(Client, Reliable)
	void Client_HideLoadingScreen();
	void Client_HideLoadingScreen_Implementation();
	UPROPERTY()
	UUserWidget* LoadingWidget;
	//----------------로딩 관련-------------------

	// 음성 녹음 관련 Server RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_NotifyRecordingStart();

	UFUNCTION(Server, Reliable)
	void ServerRPC_NotifyRecordingEnd();

	// Answer 관련 Server RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_TryStartAnswer(const FString& PlayerName);

	UFUNCTION(Server, Reliable)
	void ServerRPC_FinishAnswer();

	// 클라이언트 RPC : 토스트메시지
	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowToastMessage(const FString& Message);
	
private:
	UFUNCTION()
	void OnPlayerControlState(bool bState, class UUserWidget* FocusWidget);

private:
    class IControllable* GetControllable() const;
};
