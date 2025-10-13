// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ENetworkState : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	Waiting         UMETA(DisplayName = "Waiting"),
	Requesting      UMETA(DisplayName = "Requesting"),
	Responding      UMETA(DisplayName = "Responding"),
	Completed       UMETA(DisplayName = "Completed"),
	Failed          UMETA(DisplayName = "Failed"),
};