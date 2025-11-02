// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AYiSanPlayerState.h"
#include "Net/UnrealNetwork.h"

AYiSanPlayerState::AYiSanPlayerState()
	: PlayerIndex(-1)
{
}


void AYiSanPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AYiSanPlayerState, Nickname);
	DOREPLIFETIME(AYiSanPlayerState, PlayerIndex);
}

void AYiSanPlayerState::SetPlayerInfo(const FString& InName)
{
	if (HasAuthority())
	{
		Nickname = InName;
	}
}

void AYiSanPlayerState::SetPlayerIndex(const int32 InPlayerIndex)
{
	if (HasAuthority())
	{
		PlayerIndex = InPlayerIndex;
	}
}

void AYiSanPlayerState::OnRep_Nickname() {}

void AYiSanPlayerState::OnRep_PlayerIndex() {}