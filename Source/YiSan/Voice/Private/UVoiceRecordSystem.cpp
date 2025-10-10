// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceRecordSystem.h"
#include "UVoiceFunctionLibrary.h"
#include "GameLogging.h"


UVoiceRecordSystem::UVoiceRecordSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVoiceRecordSystem::RecordStart()
{
	if (bIsRecording)
		return;

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

	PRINTLOG(TEXT("RecordStart"));
}

void UVoiceRecordSystem::RecordStop()
{
	if (!bIsRecording)
		return;

	bIsRecording = false;
	
	AudioCapture->StopStream();
	AudioCapture->CloseStream();
	PRINTLOG(TEXT("RecordEnd"));
	
	WAVData = UVoiceFunctionLibrary::ConvertPCM2WAV(PCMData, LastSampleRate, LastNumChannels, 16);
	LastRecordedFilePath = UVoiceFunctionLibrary::SaveWavToFile(WAVData);

	OnRecordingStopped.Broadcast(LastRecordedFilePath);
}

void UVoiceRecordSystem::HandleOnCapture(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate)
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