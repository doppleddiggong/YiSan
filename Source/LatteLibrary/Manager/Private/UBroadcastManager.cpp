// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UBroadcastManger.cpp
 * @brief UBroadcastManger 구현에 대한 Doxygen 주석을 제공합니다.
 */

#include "UBroadcastManager.h"

void UBroadcastManager::SendMessage(const FString& InMsg)
{
	OnMessage.Broadcast(InMsg);
}

void UBroadcastManager::SendNetworkWaitCount(int RequestCount )
{
	OnNetworkWaitCount.Broadcast(RequestCount);
}

void UBroadcastManager::SendAudioCapture(bool bRecording)
{
	OnAudioCapture.Broadcast(bRecording);
}
//
// void UBroadcastManager::SendAnswerReply()
// {
// 	OnAnswerReply.Broadcast();
// }
//
// void UBroadcastManager::SendVoiceAudioFinished()
// {
// 	OnVoiceTalkFinished.Broadcast();
// }
//
// void UBroadcastManager::SendAskListening(bool bListening, const FString& PlayerName)
// {
// 	OnAskListening.Broadcast(bListening, PlayerName);
// }

void UBroadcastManager::SendAudioSpectrum(float Spectrum)
{
	OnAudioSpectrum.Broadcast(Spectrum);
}

void UBroadcastManager::SendContactBuilding(EBuildingType BuildingType)
{
	OnContactBuilding.Broadcast(BuildingType);	
}

void UBroadcastManager::SendUpdateQuest(EBuildingType BuildingType)
{
	OnUpdateQuest.Broadcast(BuildingType);	
}

void UBroadcastManager::SendNearBuilding(EBuildingType BuildingType)
{
	OnNearBuilding.Broadcast(BuildingType);
}

void UBroadcastManager::SendFocusBuilding(EBuildingType BuildingType)
{
	OnFocusBuilding.Broadcast(BuildingType);	
}

void UBroadcastManager::SendDoorMessage(const int32 GateID, const bool Open)
{
	OnDoorMessage.Broadcast(GateID, Open);	
}

void UBroadcastManager::SendExecVoiceCommand(const EVoiceCommandType Type, AActor* Requester)
{
	OnExecVoiceCommand.Broadcast(Type, Requester);
}

void UBroadcastManager::SendMegaPopupClosed()
{
	OnMegaPopupClosed.Broadcast();
}

void UBroadcastManager::SendPlayerControlState(bool bState, UUserWidget* FocusWidget)
{
	OnPlayerControlState.Broadcast(bState, FocusWidget);
}

void UBroadcastManager::SendHitStop(AActor* Target, const EDamageType Type)
{
	OnHitStop.Broadcast(Target, Type);
}

void UBroadcastManager::SendHitStopPair(
	AActor* Attacker, const EDamageType AttackerType,
	AActor* Target,   const EDamageType TargetType)
{
	OnHitStop.Broadcast(Attacker, AttackerType);
	OnHitStop.Broadcast(Target,   TargetType);
}

void UBroadcastManager::SendCameraShake(AActor* Target, const EDamageType Type)
{
	OnCameraShake.Broadcast(Target, Type);
}

void UBroadcastManager::SendForceCameraShake(const EDamageType Type)
{
	OnForceCameraShake.Broadcast(Type);
}

void UBroadcastManager::SendKnockback(AActor* Target, AActor* Instigator, EDamageType Type, float Resistance)
{
	OnKnockback.Broadcast(Target, Instigator, Type, Resistance);
}

// ========================================
// Multiplayer Session Events
// ========================================

void UBroadcastManager::SendSessionHost(const FString& MapName)
{
	OnSessionHost.Broadcast(MapName);
}

void UBroadcastManager::SendSessionJoin(const FString& Address, int32 Port)
{
	OnSessionJoin.Broadcast(Address, Port);
}

void UBroadcastManager::SendSessionDisconnect()
{
	OnSessionDisconnect.Broadcast();
}

void UBroadcastManager::SendSessionError(const FString& ErrorMessage)
{
	OnSessionError.Broadcast(ErrorMessage);
}