// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceConversationSystem.h"

#include "GameLogging.h"
#include "APlayerActor.h"
#include "UBroadcastManager.h"
#include "UHttpNetworkSystem.h"
#include "UVoiceFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

UVoiceConversationSystem::UVoiceConversationSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVoiceConversationSystem::InitSystem(APlayerActor* InOwner)
{
	this->Owner = InOwner;

	BroadcastManager = UBroadcastManager::Get(GetWorld());
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
	
	const bool bStreamOpened = AudioCapture->OpenAudioCaptureStream(
		Params,
		[this](const void* InAudio, int32 NumFrames, int32 InNumChannels, int32 InSampleRate, double StreamTime, bool bOverFlow)
		{
			HandleOnCapture(static_cast<const float*>(InAudio), NumFrames, InNumChannels, InSampleRate);
		},
		512
	);

	if (!bStreamOpened || !AudioCapture->StartStream() )
	{
		BroadcastManager->SendToastMessage(TEXT("연결된 마이크가 없습니다"));
		bIsRecording = false;
		return;
	}

	BroadcastManager->SendAudioCapture(true);
	PRINTLOG( TEXT("[VoiceConversation] Recording started."));
}


void UVoiceConversationSystem::HandleOnCapture(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate)
{
	LastSampleRate  = InSampleRate;
	LastNumChannels = InNumChannels;
	
	const int32 SampleCount = InNumFrames * InNumChannels;
	PCMData.Reserve(PCMData.Num() + SampleCount * sizeof(int16));

	float TotalVolume = 0.f;
	
	for (int32 i = 0; i < SampleCount; ++i)
	{
		float Sample = InAudio[i];
		Sample = FMath::Clamp(Sample, -1.0f, 1.0f);

		TotalVolume += FMath::Abs(Sample);

		int16 Int16Sample = static_cast<int16>(Sample * 32767.0f);
		const uint8* SampleBytes = reinterpret_cast<const uint8*>(&Int16Sample);

		PCMData.Append(SampleBytes, sizeof(int16));
	}

	const float AverageVolume = (SampleCount > 0) ? (TotalVolume / SampleCount) : 0.f;
	const float AmplifiedVolume = AverageVolume * 10.f;
	
	BroadcastManager->SendAudioSpectrum(FMath::Clamp(AmplifiedVolume, 0.f, 1.f));
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

	BroadcastManager->SendAudioCapture(false);
	
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

	HttpSystem->RequestASK(LastRecordedFilePath, Owner->GetGPTContext(), FResponseAskDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnResponseAsk
	));
}

void UVoiceConversationSystem::OnResponseAsk(FResponseAsk& Response, bool bSuccess)
{
	bIsProcessing = false;

	if (bSuccess)
	{
		PRINTLOG(TEXT("OnResponseAsk: Received audio data size: %d"), Response.audio_data.Num());

		BroadcastManager->SendToastMessage(Response.gpt_response_text);

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