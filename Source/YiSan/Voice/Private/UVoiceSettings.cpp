// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceSettings.h"

USpeechSettings::USpeechSettings()
{
	CategoryName = TEXT("Project");
	SectionName = TEXT("GoogleSpeechService");

	GoogleConfig.LanguageCode = TEXT("ko-KR");
	GoogleConfig.SpeechSampleRate = 16000;
	GoogleConfig.VoiceName = TEXT("ko-KR-Wavenet-A");
	GoogleConfig.VoiceSsmlGender = TEXT("FEMALE");
	GoogleConfig.TextToSpeechSampleRate = 22050;
	GoogleConfig.AudioEncoding = TEXT("LINEAR16");
}

const FVoiceConfig& USpeechSettings::Get() const
{
	return GoogleConfig;
}