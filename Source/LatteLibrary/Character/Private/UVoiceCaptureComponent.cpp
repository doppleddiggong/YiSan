// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceCaptureComponent.h"

#include "GameLogging.h"
#include "GoogleSpeechService.h"
#include "Sound/SoundWaveProcedural.h"
#include "Kismet/GameplayStatics.h"
#include "UVoiceSystem.h"
#include "VoiceModule.h"
#include "VoiceModule.h"
// #include "IVoiceCapture.h"
#include "Misc/Base64.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/Guid.h"

#include "Modules/ModuleManager.h"

namespace
{
	static const FString DefaultVoiceContextTag = TEXT("PushToTalk");
}

UVoiceCaptureComponent::UVoiceCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UVoiceCaptureComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureServices();
}

void UVoiceCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsCapturing || !VoiceCapture.IsValid())
	{
		return;
	}

	DrainVoiceCapture();
}

void UVoiceCaptureComponent::SetVoiceSystem(UVoiceSystem* InVoiceSystem)
{
	VoiceSystem = InVoiceSystem;
}

void UVoiceCaptureComponent::HandlePushToTalkPressed(const FString& ContextTag)
{
	EnsureServices();

	if (!VoiceSystem)
	{
		PRINTLOG(TEXT("[VoiceCapture] VoiceSystem missing."));
		return;
	}

	if (!EnsureVoiceCapture())
	{
		PRINTLOG(TEXT("[VoiceCapture] Failed to initialise voice capture."));
		return;
	}

	if (bIsCapturing)
	{
		return;
	}

	const FString SessionTag = ContextTag.IsEmpty() ? DefaultVoiceContextTag : ContextTag;
	VoiceSystem->StartCapture(SessionTag);
	LastCorrelationId = VoiceSystem->GetActiveCorrelationId();

	CaptureBuffer.Reset();
	ScratchBuffer.Reset();

	StartAudioCapture();
}

void UVoiceCaptureComponent::HandlePushToTalkReleased()
{
	EnsureServices();

	if (!VoiceCapture.IsValid() || !bIsCapturing)
	{
		return;
	}

	DrainVoiceCapture();
	StopAudioCapture();

	FString CorrelationId = LastCorrelationId;
	if (VoiceSystem)
	{
		const FString ActiveId = VoiceSystem->GetActiveCorrelationId();
		if (!ActiveId.IsEmpty())
		{
			CorrelationId = ActiveId;
		}
	}
	if (CorrelationId.IsEmpty())
	{
		CorrelationId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	}
	LastCorrelationId = CorrelationId;

	if (VoiceSystem)
	{
		VoiceSystem->StopCapture();
	}

	if (CaptureBuffer.Num() == 0)
	{
		PRINTLOG(TEXT("[VoiceCapture] No captured audio - skipping STT request."));
		if (VoiceSystem)
		{
			VoiceSystem->SubmitTranscription(TEXT(""), 0.0f);
		}
		return;
	}

	TArray<uint8> PayloadCopy = CaptureBuffer;
	CaptureBuffer.Reset();

	DispatchSpeechToText(CorrelationId, PayloadCopy);
}

void UVoiceCaptureComponent::RequestSynthesis(const FString& Text)
{
	EnsureServices();

	if (!SpeechService)
	{
		PRINTLOG(TEXT("[VoiceCapture] GoogleSpeechService unavailable."));
		return;
	}

	FString CorrelationId;
	if (VoiceSystem)
	{
		CorrelationId = VoiceSystem->GetActiveCorrelationId();
		if (CorrelationId.IsEmpty())
		{
			CorrelationId = LastCorrelationId;
		}
	}
	else
	{
		CorrelationId = LastCorrelationId;
	}
	if (CorrelationId.IsEmpty())
	{
		CorrelationId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	}
	LastCorrelationId = CorrelationId;
	DispatchTextToSpeech(CorrelationId, Text);
}

void UVoiceCaptureComponent::EnsureServices()
{
	if (!VoiceSystem && GetOwner())
	{
		VoiceSystem = GetOwner()->FindComponentByClass<UVoiceSystem>();
	}

	if (!SpeechService)
	{
		SpeechService = UGoogleSpeechService::Get(this);
	}
}

bool UVoiceCaptureComponent::EnsureVoiceCapture()
{
	return false;
	// if (VoiceCapture.IsValid())
	// {
	// 	return true;
	// }
	//
	// // Voice 모듈 로드 확인
	// IVoiceModule* VoiceModule = FModuleManager::GetModulePtr<IVoiceModule>("Voice");
	// if (VoiceModule == nullptr)
	// {
	// 	// 필요 시 강제 로드 (에디터/패키지 환경에 따라 선택)
	// 	if (FModuleManager::Get().ModuleExists("Voice"))
	// 	{
	// 		VoiceModule = &FModuleManager::LoadModuleChecked<IVoiceModule>("Voice");
	// 	}
	// }
	//
	// if (VoiceModule == nullptr)
	// {
	// 	PRINTLOG(TEXT("[VoiceCapture] Voice module unavailable (not loaded)."));
	// 	return false;
	// }
	//
	// VoiceCapture = VoiceModule->CreateVoiceCapture(TEXT(""), CaptureSampleRate, CaptureNumChannels);
	// if (!VoiceCapture.IsValid())
	// {
	// 	PRINTLOG(TEXT("[VoiceCapture] Failed to create voice capture stream."));
	// 	return false;
	// }
	//
	// // 디바이스 준비 확인
	// VoiceCapture->Start();
	// VoiceCapture->Stop();
	// return true;
}

void UVoiceCaptureComponent::StartAudioCapture()
{
	if (!VoiceCapture.IsValid())
	{
		return;
	}

	VoiceCapture->Start();
	bIsCapturing = true;
}

void UVoiceCaptureComponent::StopAudioCapture()
{
	if (!VoiceCapture.IsValid())
	{
		return;
	}

	VoiceCapture->Stop();
	bIsCapturing = false;
}

void UVoiceCaptureComponent::DrainVoiceCapture()
{
	if (!VoiceCapture.IsValid())
	{
		return;
	}

	uint32 AvailableBytes = 0;
	EVoiceCaptureState::Type CaptureState = VoiceCapture->GetCaptureState(AvailableBytes);
	int32 SafetyCounter = 0;

	while (CaptureState == EVoiceCaptureState::Ok && AvailableBytes > 0)
	{
		ScratchBuffer.SetNumUninitialized(AvailableBytes);
		uint32 BytesWritten = AvailableBytes;
		VoiceCapture->GetVoiceData(ScratchBuffer.GetData(), AvailableBytes, BytesWritten);
		if (BytesWritten > 0)
		{
			CaptureBuffer.Append(ScratchBuffer.GetData(), BytesWritten);
		}

		CaptureState = VoiceCapture->GetCaptureState(AvailableBytes);
		if (++SafetyCounter > 16)
		{
			break;
		}
	}
}

void UVoiceCaptureComponent::DispatchSpeechToText(const FString& CorrelationId, const TArray<uint8>& AudioPayload)
{
	if (!SpeechService)
	{
		PRINTLOG(TEXT("[VoiceCapture] Cannot dispatch STT without service."));
		return;
	}

	FGoogleSpeechToTextDelegate Delegate = FGoogleSpeechToTextDelegate::CreateUObject(this, &UVoiceCaptureComponent::HandleSpeechToTextCompleted);
	SpeechService->RequestSpeechToText(CorrelationId, AudioPayload, CaptureSampleRate, CaptureNumChannels, Delegate);
}

void UVoiceCaptureComponent::HandleSpeechToTextCompleted(bool bSuccess, const FString& CorrelationId, const FString& Transcript, float Confidence)
{
	if (VoiceSystem)
	{
		if (bSuccess)
		{
			VoiceSystem->SubmitTranscription(Transcript, Confidence);
		}
		else
		{
			VoiceSystem->SubmitTranscription(TEXT(""), 0.0f);
		}
	}

	if (bSuccess && SpeechService && !Transcript.IsEmpty())
	{
		DispatchTextToSpeech(CorrelationId, Transcript);
	}
}

void UVoiceCaptureComponent::DispatchTextToSpeech(const FString& CorrelationId, const FString& Text)
{
	if (!SpeechService || Text.IsEmpty())
	{
		return;
	}

	FGoogleTextToSpeechDelegate Delegate = FGoogleTextToSpeechDelegate::CreateUObject(this, &UVoiceCaptureComponent::HandleTextToSpeechCompleted);
	SpeechService->RequestTextToSpeech(CorrelationId, Text, 0, Delegate);
}

void UVoiceCaptureComponent::HandleTextToSpeechCompleted(bool bSuccess, const FString& CorrelationId, const FString& Text, const TArray<uint8>& AudioData, int32 SampleRate)
{
	if (VoiceSystem && bSuccess)
	{
		VoiceSystem->NotifyResponseReady(Text);
	}

	if (!bSuccess || AudioData.Num() == 0)
	{
		PRINTLOG(TEXT("[VoiceCapture] TTS failed or empty audio. CorrelationId=%s"), *CorrelationId);
		return;
	}

	USoundWaveProcedural* SoundWave = CreateSoundWaveFromPcm(AudioData, SampleRate, CaptureNumChannels);
	if (!SoundWave)
	{
		PRINTLOG(TEXT("[VoiceCapture] Failed to create sound wave for TTS."));
		return;
	}

	OnVoiceSynthesisReady.Broadcast(CorrelationId, SoundWave);
	LastCorrelationId = CorrelationId;

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SpawnSound2D(World, SoundWave);
	}
}

USoundWaveProcedural* UVoiceCaptureComponent::CreateSoundWaveFromPcm(const TArray<uint8>& AudioData, int32 SampleRate, int32 NumChannels) const
{
	if (AudioData.Num() == 0)
	{
		return nullptr;
	}

	USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>(const_cast<UVoiceCaptureComponent*>(this));
	SoundWave->SetSampleRate(SampleRate);
	SoundWave->NumChannels = NumChannels;
	SoundWave->SoundGroup = SOUNDGROUP_Voice;
	SoundWave->Duration = static_cast<float>(AudioData.Num()) / (NumChannels * sizeof(int16)) / static_cast<float>(SampleRate);
	SoundWave->bLooping = false;
	SoundWave->QueueAudio(AudioData.GetData(), AudioData.Num());

	return SoundWave;
}

