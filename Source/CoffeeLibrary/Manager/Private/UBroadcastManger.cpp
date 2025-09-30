// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UBroadcastManger.h"

void UBroadcastManger::SendMessage(const FString& InMsg)
{
	OnMessage.Broadcast(InMsg);
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