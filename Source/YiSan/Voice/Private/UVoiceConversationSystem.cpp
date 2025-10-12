// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceConversationSystem.h"

#include "GameLogging.h"
#include "UBroadcastManger.h"
#include "UHttpNetworkSystem.h"
#include "UVoiceFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

UVoiceConversationSystem::UVoiceConversationSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVoiceConversationSystem::BeginPlay()
{
	Super::BeginPlay();
	PRINTLOG(TEXT("[VoiceConversation] System initialized."));
}

void UVoiceConversationSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AudioCapture.IsValid())
		AudioCapture->CloseStream();
	Super::EndPlay(EndPlayReason);
}

// --- HTTP 방식 음성 대화 ---

void UVoiceConversationSystem::StartRecording()
{
	if (bIsRecording || bIsProcessing)
	{
		PRINTLOG( TEXT("[VoiceConversation] Already recording or processing."));
		return;
	}

	bIsRecording = true;
	PCMData.Reset();

	if (!AudioCapture)
		AudioCapture = MakeUnique<Audio::FAudioCapture>();

	Audio::FAudioCaptureDeviceParams Params;
	Params.DeviceIndex = 0;
	Params.NumInputChannels = 1;
	
	AudioCapture->OpenAudioCaptureStream(
		Params,
		[this](const void* InAudio, int32 NumFrames, int32 InNumChannels, int32 InSampleRate, double StreamTime, bool bOverFlow)
		{
			HandleOnCapture(static_cast<const float*>(InAudio), NumFrames, InNumChannels, InSampleRate);
		},
		512
	);
	
	AudioCapture->StartStream();

	PRINTLOG( TEXT("[VoiceConversation] Recording started."));
}


void UVoiceConversationSystem::HandleOnCapture(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate)
{
	LastSampleRate  = InSampleRate;
	LastNumChannels = InNumChannels;
	
	const int32 SampleCount = InNumFrames * InNumChannels;
	PCMData.Reserve(PCMData.Num() + SampleCount * sizeof(int16));

	for (int32 i = 0; i < SampleCount; ++i)
	{
		float Sample = InAudio[i];
		Sample = FMath::Clamp(Sample, -1.0f, 1.0f);

		int16 Int16Sample = static_cast<int16>(Sample * 32767.0f);
		const uint8* SampleBytes = reinterpret_cast<const uint8*>(&Int16Sample);

		PCMData.Append(SampleBytes, sizeof(int16));
	}
}

void UVoiceConversationSystem::StopRecording()
{
	if (!bIsRecording)
	{
		PRINTLOG( TEXT("[VoiceConversation] Not currently recording."));
		return;
	}

	bIsRecording = false;
	bIsProcessing = true;

	AudioCapture->StopStream();
	AudioCapture->CloseStream();
	
	WAVData = UVoiceFunctionLibrary::ConvertPCM2WAV(PCMData, LastSampleRate, LastNumChannels, 16);
	LastRecordedFilePath = UVoiceFunctionLibrary::SaveWavToFile(WAVData);

	PRINTLOG( TEXT("[VoiceConversation] Recording stopped. Processing...") );

	PRINTLOG( TEXT("[VoiceConversation] Recording saved to: %s"), *LastRecordedFilePath);

	if (LastRecordedFilePath.IsEmpty())
	{
		PRINTLOG( TEXT("[VoiceConversation] FilePath is Empty") );
		return;
	}
	
	// STT 요청
	UHttpNetworkSystem* HttpSystem = UHttpNetworkSystem::Get(GetWorld());
	if (!HttpSystem)
	{
		PRINTLOG( TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	HttpSystem->RequestAsk(LastRecordedFilePath, FResponseAskDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnResponseAsk
	));
}

void UVoiceConversationSystem::OnResponseAsk(FResponseAsk& Response, bool bSuccess)
{
	bIsProcessing = false;

	if (bSuccess)
	{
		PRINTLOG(TEXT("OnResponseAsk: Received audio data size: %d"), Response.audio_data.Num());

		if (auto EventManager = UBroadcastManger::Get(this))
			EventManager->SendToastMessage(Response.gpt_response_text);

		if (Response.audio_data.Num() == 0)
		{
			PRINTLOG(TEXT("OnResponseAsk: Audio data is empty. Cannot play TTS audio."));
			return;
		}
		
		auto SoundWave = UVoiceFunctionLibrary::CreateProceduralSoundWaveFromWavData(Response.audio_data);
		if ( IsValid(SoundWave))
			UGameplayStatics::PlaySound2D(this, SoundWave);
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
	}
}