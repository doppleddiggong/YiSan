// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/UniquePtr.h"
#include "HAL/IConsoleManager.h"
#include "UVoiceManager.generated.h"

/**
 * @brief Registers developer console commands for voice features (STT/TTS).
 */
UCLASS()
class YISAN_API UVoiceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UVoiceManager);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void HandleSpeakCommand(const TArray<FString>& Args, UWorld* World);

private:
	TUniquePtr<FAutoConsoleCommandWithWorldAndArgs> SpeakCommand;
};
