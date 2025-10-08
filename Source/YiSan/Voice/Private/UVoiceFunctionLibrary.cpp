// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceFunctionLibrary.h"
#include "GameLogging.h"

#define VOICE_LOG TEXT("VoiceLogs")

TArray<uint8> UVoiceFunctionLibrary::ConvertPCM2WAV(
	const TArray<uint8>& InPCMData,
	const int32 InSampleRate,
	const int32 InChannel,
	const int32 InBitsPerSample)
{
	TArray<uint8> WavData;

	const int32 ByteRate   = InSampleRate * InChannel * InBitsPerSample / 8;
	const int32 BlockAlign = InChannel * InBitsPerSample / 8;
	const int32 DataSize   = InPCMData.Num();
	const int32 ChunkSize  = 36 + DataSize;

	// RIFF 헤더
	WavData.Append(reinterpret_cast<const uint8*>("RIFF"), 4);
	WavData.Append(reinterpret_cast<const uint8*>(&ChunkSize), 4);
	WavData.Append(reinterpret_cast<const uint8*>("WAVE"), 4);

	// fmt chunk
	WavData.Append(reinterpret_cast<const uint8*>("fmt "), 4);
	int32 SubChunk1Size = 16;
	int16 AudioFormat   = 1;

	WavData.Append(reinterpret_cast<const uint8*>(&SubChunk1Size), 4);
	WavData.Append(reinterpret_cast<const uint8*>(&AudioFormat), 2);
	WavData.Append(reinterpret_cast<const uint8*>(&InChannel), 2);
	WavData.Append(reinterpret_cast<const uint8*>(&InSampleRate), 4);
	WavData.Append(reinterpret_cast<const uint8*>(&ByteRate), 4);
	WavData.Append(reinterpret_cast<const uint8*>(&BlockAlign), 2);
	WavData.Append(reinterpret_cast<const uint8*>(&InBitsPerSample), 2);
	WavData.Append(reinterpret_cast<const uint8*>("data"), 4);
	WavData.Append(reinterpret_cast<const uint8*>(&DataSize), 4);

	WavData.Append(InPCMData);
	return WavData;
}


void UVoiceFunctionLibrary::SaveWavToFile(TArray<uint8>& InWavData, const FString& InFileName )
{
	if (InWavData.Num() == 0)
	{
		PRINTLOG( TEXT("WavData is empty, nothing to save."));
		return;
	}

	FString FileName = InFileName;
	if (FileName.IsEmpty())
	{
		// 날짜 기반 파일명 생성
		const FDateTime Now = FDateTime::Now();
		FileName = FString::Printf(TEXT("Voice_%04d%02d%02d_%02d%02d%02d.wav"),
			Now.GetYear(), Now.GetMonth(), Now.GetDay(),
			Now.GetHour(), Now.GetMinute(), Now.GetSecond()
		);
	}

	FString FolderPath = FPaths::ProjectSavedDir() / VOICE_LOG;
	// 폴더 없으면 생성
	IFileManager::Get().MakeDirectory(*FolderPath, true);

	FString FullPath = FolderPath / FileName;
	if (FFileHelper::SaveArrayToFile(InWavData, *FullPath))
		PRINTLOG( TEXT("Saved WAV file: %s"), *FullPath);
	else
		PRINTLOG( TEXT("Failed to save WAV file: %s"), *FullPath);
	
	//
	// if (InWavData.Num() >= 4)
	// {
	// 	FString Header = FString::Printf(TEXT("%c%c%c%c"),
	// 		InWavData[0], InWavData[1], InWavData[2], InWavData[3] );
	// 	PRINTLOG( TEXT("WAV Header: %s"), *Header);
	// }	
}