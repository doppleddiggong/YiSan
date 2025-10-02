// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "GoogleSpeechServiceSettings.h"

UGoogleSpeechServiceSettings::UGoogleSpeechServiceSettings()
{
	CategoryName = TEXT("Project");
	SectionName = TEXT("GoogleSpeechService");

	LanguageCode = TEXT("ko-KR");
	SpeechSampleRate = 16000;
	VoiceName = TEXT("ko-KR-Wavenet-A");
	VoiceSsmlGender = TEXT("FEMALE");
	TextToSpeechSampleRate = 22050;
	AudioEncoding = TEXT("LINEAR16");
}

const UGoogleSpeechServiceSettings* UGoogleSpeechServiceSettings::Get()
{
	return GetDefault<UGoogleSpeechServiceSettings>();
}
