// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AYiSanPlayerState.h"
#include "Net/UnrealNetwork.h"

/**
 * @file AYiSanPlayerState.cpp
 * @brief AYiSanPlayerState의 동작을 구현합니다.
 */

/** @brief 플레이어 상태의 기본 복제 값을 초기화합니다. */
AYiSanPlayerState::AYiSanPlayerState()
        : PlayerIndex(-1)
{
}

/** @brief 복제에 사용할 닉네임과 플레이어 인덱스를 등록합니다. */
void AYiSanPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AYiSanPlayerState, Nickname);
	DOREPLIFETIME(AYiSanPlayerState, PlayerIndex);
}

/** @brief 권한 측 복제 닉네임을 갱신합니다. */
void AYiSanPlayerState::SetPlayerInfo(const FString& InName)
{
	if (HasAuthority())
	{
		Nickname = InName;
	}
}

/** @brief 서버 권한에서 지속 플레이어 슬롯을 지정합니다. */
void AYiSanPlayerState::SetPlayerIndex(const int32 InPlayerIndex)
{
	if (HasAuthority())
	{
		PlayerIndex = InPlayerIndex;
	}
}

/** @brief 닉네임이 클라이언트에서 갱신될 때 호출됩니다. */
void AYiSanPlayerState::OnRep_Nickname() {}

/** @brief 플레이어 인덱스 값이 복제될 때 호출됩니다. */
void AYiSanPlayerState::OnRep_PlayerIndex() {}