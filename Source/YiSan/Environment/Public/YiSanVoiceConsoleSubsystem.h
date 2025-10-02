// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/UniquePtr.h"
#include "YiSanVoiceConsoleSubsystem.generated.h"

struct FAutoConsoleCommandWithWorldAndArgs;

/**
 * @brief Registers developer console commands for voice features (STT/TTS).
 */
UCLASS()
class UYiSanVoiceConsoleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void HandleSpeakCommand(const TArray<FString>& Args, UWorld* World);

private:
	TUniquePtr<FAutoConsoleCommandWithWorldAndArgs> SpeakCommand;
};
