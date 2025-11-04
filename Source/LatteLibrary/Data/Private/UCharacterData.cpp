// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UCharacterData.cpp
 * @brief UCharacterData의 동작을 구현합니다.
 */
#include "UCharacterData.h"
#include "GameLogging.h"
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
