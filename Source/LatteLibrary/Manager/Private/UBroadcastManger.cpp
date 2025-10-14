// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UBroadcastManger.cpp
 * @brief UBroadcastManger 구현에 대한 Doxygen 주석을 제공합니다.
 */
#include "UBroadcastManger.h"

void UBroadcastManger::SendMessage(const FString& InMsg)
{
	OnMessage.Broadcast(InMsg);
}

void UBroadcastManger::SendToastMessage(const FString& InMsg)
{
	OnToastMessage.Broadcast(InMsg);
}

void UBroadcastManger::SendNetworkWaitCount(int RequestCount )
{
	OnNetworkWaitCount.Broadcast(RequestCount);
}

void UBroadcastManger::SendAudioCapture(bool bRecording)
{
	OnAudioCapture.Broadcast(bRecording);
}

void UBroadcastManger::SendAudioSpectrum(float Spectrum)
{
	OnAudioSpectrum.Broadcast(Spectrum);
}

void UBroadcastManger::SendFocusBuilding(EBuildingType BuildingType)
{
	OnFocusBuilding.Broadcast(BuildingType);	
}

void UBroadcastManger::SendContactBuilding(EBuildingType BuildingType)
{
	OnContactBuilding.Broadcast(BuildingType);	
}

void UBroadcastManger::SendOnDoorMessage(const int32 GateID, const bool Open)
{
	OnDoorMessage.Broadcast(GateID, Open);	
}

void UBroadcastManger::SendHitStop(AActor* Target, const EDamageType Type)
{
	OnHitStop.Broadcast(Target, Type);
}

void UBroadcastManger::SendHitStopPair(
	AActor* Attacker, const EDamageType AttackerType,
	AActor* Target,   const EDamageType TargetType)
{
	OnHitStop.Broadcast(Attacker, AttackerType);
	OnHitStop.Broadcast(Target,   TargetType);
}

void UBroadcastManger::SendCameraShake(AActor* Target, const EDamageType Type)
{
	OnCameraShake.Broadcast(Target, Type);
}

void UBroadcastManger::SendForceCameraShake(const EDamageType Type)
{
	OnForceCameraShake.Broadcast(Type);
}

void UBroadcastManger::SendKnockback(AActor* Target, AActor* Instigator, EDamageType Type, float Resistance)
{
	OnKnockback.Broadcast(Target, Instigator, Type, Resistance);
}