// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AYiSanPlayerState.h"
#include "Net/UnrealNetwork.h"

AYiSanPlayerState::AYiSanPlayerState()
{
	PlayerIndex = -1;
}

void AYiSanPlayerState::SetPlayerInfo(const FString& InName, int32 InIndex)

{

	if (HasAuthority())

	{

		Nickname = InName;

		PlayerIndex = InIndex;

	}

}



void AYiSanPlayerState::OnRep_Nickname() {}



void AYiSanPlayerState::OnRep_PlayerIndex() {}

void AYiSanPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AYiSanPlayerState, Nickname);
	DOREPLIFETIME(AYiSanPlayerState, PlayerIndex);
}