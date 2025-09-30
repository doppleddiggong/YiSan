// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCharacterData.h"
#include "Shared/GameLogging.h"
#include "Animation/AnimMontage.h"

bool UCharacterData::LoadIdleMontage(TSoftObjectPtr<UAnimMontage>& OutMontage) const
{
	OutMontage = IdleAsset.LoadSynchronous();
	if (!OutMontage)
	{
		PRINTLOG(TEXT("Failed to LoadWinMontage"));
		return false;
	}
	return true;
}
