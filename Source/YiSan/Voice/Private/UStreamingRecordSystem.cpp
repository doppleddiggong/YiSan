// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStreamingRecordSystem.h"
#include "GameLogging.h"
#include "Logging/LogMacros.h"

UStreamingRecordSystem::UStreamingRecordSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStreamingRecordSystem::StartStreaming()
{
	if (bIsRecording)
		return;

	bIsRecording = true;

	if (!AudioCapture)
		AudioCapture = MakeUnique<Audio::FAudioCapture>();

	Audio::FAudioCaptureDeviceParams Params;
	Params.DeviceIndex = 0;
	Params.NumInputChannels = 1;
	
	bool bOpenSuccess = AudioCapture->OpenAudioCaptureStream(
		Params,
		[this](const void* InAudio, int32 NumFrames, int32 InNumChannels, int32 InSampleRate, double StreamTime, bool bOverFlow)
		{
			HandleOnCapture(static_cast<const float*>(InAudio), NumFrames, InNumChannels, InSampleRate);
		},
		512
	);

	if (!bOpenSuccess)
	{
		NETWORK_LOG(TEXT("StartStreaming: Failed to open audio capture stream."));
		bIsRecording = false;
		return;
	}
		
	bool bStartSuccess = AudioCapture->StartStream();

	if (!bStartSuccess)
	{
		NETWORK_LOG(TEXT("StartStreaming: Failed to start audio capture stream."));

		bIsRecording = false;
		AudioCapture->CloseStream();
		return;
	}

	NETWORK_LOG(TEXT("StartStreaming"));
}

void UStreamingRecordSystem::StopStreaming()
{
	if (!bIsRecording)
		return;

	bIsRecording = false;
	
	AudioCapture->StopStream();
	AudioCapture->CloseStream();
	
	NETWORK_LOG(TEXT("StopStreaming"));
}

void UStreamingRecordSystem::HandleOnCapture(const float* InAudio, const int32 InNumFrames, const int32 InNumChannels, const int32 InSampleRate)
{
	LastSampleRate  = InSampleRate;
	LastNumChannels = InNumChannels;
	
	const int32 SampleCount = InNumFrames * InNumChannels;
	const int32 ChunkDataSize = SampleCount * sizeof(int16);

	TArray<uint8> PcmChunk;
	PcmChunk.Reserve(ChunkDataSize);

	for (int32 i = 0; i < SampleCount; ++i)
	{
		float Sample = InAudio[i];
		Sample = FMath::Clamp(Sample, -1.0f, 1.0f);

		int16 Int16Sample = static_cast<int16>(Sample * 32767.0f);
		const uint8* SampleBytes = reinterpret_cast<const uint8*>(&Int16Sample);

		PcmChunk.Append(SampleBytes, sizeof(int16));
	}

	// NETWORK_LOG( TEXT("HandleOnCapture called. PcmChunk size: %d"), PcmChunk.Num());

	if (OnAudioChunkReady.IsBound())
	{
		OnAudioChunkReady.Broadcast(PcmChunk);
	}
}