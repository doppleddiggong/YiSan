// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file APlayerControl.h
 * @brief APlayerControl 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "APlayerControl.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple))
class LATTELIBRARY_API APlayerControl : public APlayerController
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
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Record;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_ShowDetail;
	
	// --- Handlers ---
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnAltitudeUp(const FInputActionValue& Value);
	void OnAltitudeDown(const FInputActionValue& Value);
	void OnAltitudeReleased(const FInputActionValue& Value);
	
	void OnJump(const FInputActionValue& Value);
	void OnLanding(const FInputActionValue& Value);

	void OnChat(const FInputActionValue& Value);
	void OnRecordPressed(const FInputActionValue& Value);
	void OnRecordReleased(const FInputActionValue& Value);

	void OnShowDetail(const FInputActionValue& Value);

public:
	// ========================================
	// Multiplayer Session Functions (UI Binding)
	// ========================================
	// 주의: 이 함수들은 MultiplayerSessionComponent로 위임됩니다.

	/// @brief 호스트로 게임 세션을 생성합니다 (UI 위젯에서 호출).
	/// @param MapName 호스팅할 맵 이름
	/// @param MaxPlayers 최대 플레이어 수
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	void HostSession(const FString& MapName = TEXT("MainLevel_WP"), int32 MaxPlayers = 4);

	/// @brief 기존 게임 세션에 참가합니다 (UI 위젯에서 호출).
	/// @param Address 서버 IP 주소 또는 도메인
	/// @param Port 서버 포트
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	void JoinSession(const FString& Address, int32 Port = 7777);

	/// @brief 현재 세션에서 연결을 끊습니다 (UI 위젯에서 호출).
	UFUNCTION(BlueprintCallable, Category="Multiplayer|Session")
	void DisconnectSession();

private:
	UFUNCTION()
	void OnPlayerControlState(bool bState, class UUserWidget* FocusWidget);

private:
    class IControllable* GetControllable() const;
};
