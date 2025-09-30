// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EGameSoundType.h"
#include "Engine/DataAsset.h"
#include "USoundData.generated.h"


UCLASS()
class LATTELIBRARY_API USoundData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Sound")
	TMap<EGameSoundType, TObjectPtr<class USoundBase>> SoundData;
};
