// Copyright Epic Games, Inc. All Rights Reserved.

#include "UYiSanVoiceConsoleSubsystem.h"
#include "UVoiceCaptureComponent.h"

#include "GameLogging.h"
#include "Templates/UniquePtr.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "APlayerActor.h"
#include "HAL/IConsoleManager.h"

void UYiSanVoiceConsoleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SpeakCommand = MakeUnique<FAutoConsoleCommandWithWorldAndArgs>(
		TEXT("yisan.tts"),
		TEXT("yisan.tts <text> - synthesize text via Google TTS and play through the player's voice component."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateUObject(
			this, &UYiSanVoiceConsoleSubsystem::HandleSpeakCommand));
}

void UYiSanVoiceConsoleSubsystem::Deinitialize()
{
	SpeakCommand.Reset();
	Super::Deinitialize();
}

void UYiSanVoiceConsoleSubsystem::HandleSpeakCommand(const TArray<FString>& Args, UWorld* World)
{
	if (!World)
	{
		PRINTLOG( TEXT("yisan.tts called without a valid world."));
		return;
	}

	if (Args.Num() == 0)
	{
		PRINTLOG( TEXT("yisan.tts requires text input."));
		return;
	}

	FString Text = FString::Join(Args, TEXT(" "));
	Text.TrimStartAndEndInline();

	if (Text.IsEmpty())
	{
		PRINTLOG( TEXT("yisan.tts ignored empty text."));
		return;
	}

	for (TActorIterator<APlayerActor> It(World); It; ++It)
	{
		if (APlayerActor* Player = *It)
		{
			if (UVoiceCaptureComponent* VoiceComponent = Player->FindComponentByClass<UVoiceCaptureComponent>())
			{
				VoiceComponent->RequestSynthesis(Text);
				PRINTLOG( TEXT("yisan.tts dispatched text (%s)."), *Text);
				return;
			}
		}
	}

	PRINTLOG( TEXT("yisan.tts could not find an APlayerActor with UVoiceCaptureComponent."));
}
