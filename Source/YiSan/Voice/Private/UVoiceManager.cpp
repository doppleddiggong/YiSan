// Copyright Epic Games, Inc. All Rights Reserved.

#include "UVoiceManager.h"
#include "UAudioCaptureComponent.h"

#include "GameLogging.h"
#include "Templates/UniquePtr.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"

void UVoiceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SpeakCommand = MakeUnique<FAutoConsoleCommandWithWorldAndArgs>(
		TEXT("voice.tts"),
		TEXT("voice.tts <text> - synthesize text via Google TTS and play through the player's voice component."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateUObject(
			this, &UVoiceManager::HandleSpeakCommand));
}

void UVoiceManager::Deinitialize()
{
	SpeakCommand.Reset();
	Super::Deinitialize();
}

void UVoiceManager::HandleSpeakCommand(const TArray<FString>& Args, UWorld* World)
{
	if (!World)
	{
		PRINTLOG( TEXT("voice.tts called without a valid world."));
		return;
	}

	if (Args.Num() == 0)
	{
		PRINTLOG( TEXT("voice.tts requires text input."));
		return;
	}

	FString Text = FString::Join(Args, TEXT(" "));
	Text.TrimStartAndEndInline();

	if (Text.IsEmpty())
	{
		PRINTLOG( TEXT("voice.tts ignored empty text."));
		return;
	}

	// for (TActorIterator<APlayerActor> It(World); It; ++It)
	// {
	// 	if (APlayerActor* Player = *It)
	// 	{
	// 		if (UVoiceCaptureComponent* VoiceComponent = Player->FindComponentByClass<UVoiceCaptureComponent>())
	// 		{
	// 			VoiceComponent->RequestSynthesis(Text);
	// 			PRINTLOG( TEXT("yisan.tts dispatched text (%s)."), *Text);
	// 			return;
	// 		}
	// 	}
	// }

	PRINTLOG( TEXT("voice.tts could not find an APlayerActor with UVoiceCaptureComponent."));
}
