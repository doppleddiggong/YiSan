// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "YiSanGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class YISAN_API UYiSanGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level") FName TargetLevel = TEXT("MainLevel_WP");
		UPROPERTY(BlueprintReadOnly) bool bLevelReady = false;
};
