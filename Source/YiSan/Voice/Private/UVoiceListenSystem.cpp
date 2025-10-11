// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UVoiceListenSystem.h"

#include "GameLogging.h"
#include "UVoiceFunctionLibrary.h"
#include "UWebSocketSystem.h"
#include "Sound/SoundWaveProcedural.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UVoiceListenSystem::UVoiceListenSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVoiceListenSystem::InitSystem()
{
	// 오디오 컴포넌트 생성
	AudioComponent = NewObject<UAudioComponent>(this);
	AudioComponent->RegisterComponent();
	
	if (auto WebSocketSystem = UWebSocketSystem::Get(GetWorld()))
	{
		WebSocketSystem->OnAudioStart.AddDynamic(this, &UVoiceListenSystem::HandleAudioStart);
		WebSocketSystem->OnAudioChunkReceived.AddDynamic(this, &UVoiceListenSystem::HandleAudioChunk);
		WebSocketSystem->OnAudioEnd.AddDynamic(this, &UVoiceListenSystem::HandleAudioEnd);
	}
}

void UVoiceListenSystem::HandleAudioStart()
{
	UE_LOG(LogTemp, Warning, TEXT("HandleAudioStart: 오디오 스트림 시작. 재생을 준비합니다."));

	ProceduralSoundWave = NewObject<USoundWaveProcedural>();
	if (ProceduralSoundWave)
	{
		// 서버에서 보내주는 오디오 포맷에 맞춰야 합니다. (예: 16kHz, 모노)
		ProceduralSoundWave->SetSampleRate(16000);
		ProceduralSoundWave->NumChannels = 1;
		ProceduralSoundWave->Duration = INDEFINITELY_LOOPING_DURATION; // 스트리밍이므로 길이를 무한으로 설정
		ProceduralSoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Voice;
		ProceduralSoundWave->bLooping = false;

		if (AudioComponent)
		{
			AudioComponent->SetSound(ProceduralSoundWave);
			AudioComponent->Play();
		}
	}
}

void UVoiceListenSystem::HandleAudioChunk(const TArray<uint8>& AudioData)
{
	if (ProceduralSoundWave && AudioComponent && AudioComponent->IsPlaying())
	{
		// 받은 오디오 데이터 조각을 사운드 웨이브의 큐에 추가합니다.
		ProceduralSoundWave->QueueAudio(AudioData.GetData(), AudioData.Num());
	}
}

void UVoiceListenSystem::HandleAudioEnd()
{
	PRINTLOG( TEXT("HandleAudioEnd: 오디오 스트림이 종료되었습니다."));
	// 큐에 데이터가 더 이상 추가되지 않으면 재생이 자연스럽게 멈춥니다.
}

void UVoiceListenSystem::HandleTTSOutput(const TArray<uint8>& AudioData)
{
	const FString SavePath = FPaths::ProjectSavedDir() / TEXT("TTS_Output.wav");

	if (FFileHelper::SaveArrayToFile(AudioData, *SavePath))
	{
		PRINTLOG(TEXT("TTS WAV 저장 완료: %s"), *SavePath);

		// 언리얼에서 재생하려면 SoundWave로 로드
		USoundWave* Sound = UVoiceFunctionLibrary::CreateSoundWaveFromWavData(AudioData);

		if (Sound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), Sound);
		}
		else
		{
			PRINTLOG(TEXT("SoundWave 생성 실패"));
		}
	}
	else
	{
		PRINTLOG(TEXT("TTS WAV 저장 실패"));
	}
}