// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UVoiceFunctionLibrary.generated.h"

UCLASS()
class YISAN_API UVoiceFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Voice|Utility")
	static TArray<uint8> ConvertPCM2WAV(
		const TArray<uint8>& InPCMData,
		int32 InSampleRate,
		int32 InChannel,
		int32 InBitsPerSample
	);

	UFUNCTION(BlueprintCallable, Category="Voice|Utility")
	static void SaveWavToFile(TArray<uint8>& InWavData,
		const FString& InFileName = TEXT(""));
};
