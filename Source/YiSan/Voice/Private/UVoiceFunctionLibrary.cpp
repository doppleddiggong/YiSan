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


FString UVoiceFunctionLibrary::SaveWavToFile(TArray<uint8>& InWavData, const FString& InFileName )
{
	if (InWavData.Num() == 0)
	{
		PRINTLOG( TEXT("WavData is empty, nothing to save."));
		return FString();
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
	{
		PRINTLOG( TEXT("Saved WAV file: %s"), *FullPath);
		return FullPath;
	}
	else
	{
		PRINTLOG( TEXT("Failed to save WAV file: %s"), *FullPath);
		return FString();
	}
}







static uint32 ReadUInt32(const uint8* Data, int32 Offset)
{
    return Data[Offset] |
           (Data[Offset + 1] << 8) |
           (Data[Offset + 2] << 16) |
           (Data[Offset + 3] << 24);
}

static uint16 ReadUInt16(const uint8* Data, int32 Offset)
{
    return Data[Offset] | (Data[Offset + 1] << 8);
}

USoundWave* UVoiceFunctionLibrary::CreateSoundWaveFromWavData(const TArray<uint8>& WavData)
{
    if (WavData.Num() < 44)
    {
        PRINTLOG( TEXT("Invalid WAV data (too small)"));
        return nullptr;
    }

    const uint8* RawData = WavData.GetData();

    // WAV Header Parsing
    // ChunkID "RIFF" (0~3)
    // Format "WAVE" (8~11)
    // Subchunk1ID "fmt " (12~15)
    // AudioFormat, NumChannels, SampleRate, ByteRate, BlockAlign, BitsPerSample
    uint16 AudioFormat  = ReadUInt16(RawData, 20);
    uint16 NumChannels  = ReadUInt16(RawData, 22);
    uint32 SampleRate   = ReadUInt32(RawData, 24);
    uint16 BitsPerSample = ReadUInt16(RawData, 34);

    // Subchunk2ID "data"는 고정 위치가 아니므로 탐색
    int32 DataChunkOffset = 36;
    while (DataChunkOffset + 8 < WavData.Num())
    {
        uint32 ChunkID =
            RawData[DataChunkOffset] |
            (RawData[DataChunkOffset + 1] << 8) |
            (RawData[DataChunkOffset + 2] << 16) |
            (RawData[DataChunkOffset + 3] << 24);

        uint32 ChunkSize = ReadUInt32(RawData, DataChunkOffset + 4);

        // 'data' 체크
        if (ChunkID == 'atad') // 'data'를 리틀엔디언 'atad'로 읽음
        {
            break;
        }

        // 다음 Chunk로 이동
        DataChunkOffset += (8 + ChunkSize);
    }

    if (DataChunkOffset + 8 >= WavData.Num())
    {
        PRINTLOG( TEXT("WAV data chunk not found"));
        return nullptr;
    }

    const int32 DataStart = DataChunkOffset + 8;
    const int32 DataSize = WavData.Num() - DataStart;

    // SoundWave 생성
    USoundWave* SoundWave = NewObject<USoundWave>();
    if (!SoundWave)
    {
        PRINTLOG( TEXT("Failed to create USoundWave"));
        return nullptr;
    }

    SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Voice;
    SoundWave->DecompressionType = EDecompressionType::DTYPE_Procedural;
    SoundWave->bLooping = false;

    SoundWave->NumChannels   = NumChannels;
    SoundWave->Duration      = (float)DataSize / (SampleRate * NumChannels * (BitsPerSample / 8));
    SoundWave->SetSampleRate(SampleRate);
    SoundWave->RawPCMDataSize = DataSize;

    // RawPCMData에 복사
    uint8* PCMData = (uint8*)FMemory::Malloc(DataSize);
    FMemory::Memcpy(PCMData, RawData + DataStart, DataSize);
    SoundWave->RawPCMData = PCMData;

    return SoundWave;
}