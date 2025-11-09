// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UBroadcastManger.h
 * @brief UBroadcastManger 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "EDamageType.h"
#include "EBuildingType.h"
#include "EVoiceCommandType.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UBroadcastManager.generated.h"

/**
 * @brief 게임 내 전역 이벤트를 중계하는 중앙 이벤트 버스(Event Bus) 서브시스템입니다.
 * @details 이 매니저는 델리게이트를 사용하여 시스템 간의 직접적인 종속성을 제거하고,
 *          느슨한 결합(Loose Coupling)을 통해 유연하고 확장 가능한 아키텍처를 지원합니다.
 *          각 서브시스템은 이 매니저를 통해 이벤트를 보내거나(Send) 구독(Bind)할 수 있습니다.
 */
UCLASS()
class LATTELIBRARY_API UBroadcastManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UBroadcastManager);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessage, FString, Msg);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnMessage OnMessage;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendMessage(const FString& InMsg);
	//
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToastMessage, FString, Msg);
	// UPROPERTY(BlueprintAssignable, Category="Events")
	// FOnToastMessage OnToastMessage;
	//
	// UFUNCTION(BlueprintCallable, Category="Events")
	// void SendToastMessage(const FString& InMsg);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkWaitCount, int, RequestCount);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnNetworkWaitCount OnNetworkWaitCount;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendNetworkWaitCount(int RequestCount);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioCapture, bool, bRecording);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnAudioCapture OnAudioCapture;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendAudioCapture(bool bRecording);

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnswerReply);
	// UPROPERTY(BlueprintAssignable, Category="Events")
	// FOnAnswerReply OnAnswerReply;
	//
	// UFUNCTION(BlueprintCallable, Category="Events")
	// void SendAnswerReply();

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVoiceAutioFinished);
	// UPROPERTY(BlueprintAssignable, Category="Events")
	// FOnVoiceAutioFinished OnVoiceTalkFinished;
	//
	// UFUNCTION(BlueprintCallable, Category="Events")
	// void SendVoiceAudioFinished();

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDasanListening, bool, bListening, FString, PlayerName);
	// UPROPERTY(BlueprintAssignable, Category="Events")
	// FOnDasanListening OnAskListening;
	//
	// UFUNCTION(BlueprintCallable, Category="Events")
	// void SendAskListening(bool bListening, const FString& PlayerName);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioSpectrum, float, Spectrum);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnAudioSpectrum OnAudioSpectrum;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendAudioSpectrum(float Spectrum);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContactBuilding, EBuildingType, BuildingType);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnContactBuilding OnContactBuilding;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendContactBuilding(EBuildingType BuildingType);
		
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateQuest, EBuildingType, NewTarget);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnUpdateQuest OnUpdateQuest;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendUpdateQuest(EBuildingType BuildingType);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNearBuilding, EBuildingType, BuildingType);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnNearBuilding OnNearBuilding;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendNearBuilding(EBuildingType BuildingType);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusBuilding, EBuildingType, BuildingType);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnFocusBuilding OnFocusBuilding;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendFocusBuilding(EBuildingType BuildingType);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDoorMessage, int32, GateID, bool, Open );
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnDoorMessage OnDoorMessage;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendDoorMessage(const int32 GateID, const bool Open);

	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExecVoiceCommand, EVoiceCommandType, Type, AActor*, Requester);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnExecVoiceCommand OnExecVoiceCommand;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendExecVoiceCommand(const EVoiceCommandType Type, AActor* Requester);


	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMegaPopupClosed);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnMegaPopupClosed OnMegaPopupClosed;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendMegaPopupClosed();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerControlState, bool, bState, UUserWidget*, FocusWidget);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnPlayerControlState OnPlayerControlState;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendPlayerControlState(bool bState, UUserWidget* FocusWidget);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitStop, AActor*, Target, EDamageType, Type);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnHitStop OnHitStop;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendHitStop(AActor* Target, const EDamageType Type);

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendHitStopPair(AActor* Attacker, const EDamageType AttackerType,
						 AActor* Target,   const EDamageType TargetType);

	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCameraShake, AActor*, Target, EDamageType, Type);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnCameraShake OnCameraShake;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendCameraShake(AActor* Target, const EDamageType Type);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnForceCameraShake, EDamageType, Type);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnForceCameraShake OnForceCameraShake;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendForceCameraShake(const EDamageType Type);

	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnKnockback,
		AActor*,		   Target,
		AActor*,           Instigator,
		EDamageType,  Type,
		float,             Resistance );
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnKnockback OnKnockback;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendKnockback(AActor* Target, AActor* Instigator, EDamageType Type, float Resistance);

	// ========================================
	// Multiplayer Session Events
	// ========================================

	/// @brief 세션 호스트 생성 이벤트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionHost, FString, MapName);
	UPROPERTY(BlueprintAssignable, Category="Events|Multiplayer")
	FOnSessionHost OnSessionHost;

	UFUNCTION(BlueprintCallable, Category="Events|Multiplayer")
	void SendSessionHost(const FString& MapName);

	/// @brief 세션 참가 이벤트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionJoin, FString, Address, int32, Port);
	UPROPERTY(BlueprintAssignable, Category="Events|Multiplayer")
	FOnSessionJoin OnSessionJoin;

	UFUNCTION(BlueprintCallable, Category="Events|Multiplayer")
	void SendSessionJoin(const FString& Address, int32 Port);

	/// @brief 세션 연결 해제 이벤트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionDisconnect);
	UPROPERTY(BlueprintAssignable, Category="Events|Multiplayer")
	FOnSessionDisconnect OnSessionDisconnect;

	UFUNCTION(BlueprintCallable, Category="Events|Multiplayer")
	void SendSessionDisconnect();

	/// @brief 세션 오류 이벤트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionError, FString, ErrorMessage);
	UPROPERTY(BlueprintAssignable, Category="Events|Multiplayer")
	FOnSessionError OnSessionError;

	UFUNCTION(BlueprintCallable, Category="Events|Multiplayer")
	void SendSessionError(const FString& ErrorMessage);
};
