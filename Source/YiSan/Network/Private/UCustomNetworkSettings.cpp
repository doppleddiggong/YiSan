// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UCustomNetworkSettings.cpp
 * @brief UCustomNetworkSettings의 동작을 구현합니다.
 */
#include "UCustomNetworkSettings.h"
#include "Misc/CommandLine.h"

EServerMode UCustomNetworkSettings::GetCurrentServerMode()
{
	FString CmdMode;
	if (FParse::Value(FCommandLine::Get(), TEXT("ServerMode="), CmdMode))
	{
		if (CmdMode.Equals("Stage", ESearchCase::IgnoreCase))
		{
			return EServerMode::Stage;
		}
		if (CmdMode.Equals("Live", ESearchCase::IgnoreCase))
		{
			return EServerMode::Live;
		}
	}
	
	return GetDefault<UCustomNetworkSettings>()->DefaultMode;
}

const FServerConfig& UCustomNetworkSettings::GetConfig(EServerMode Mode) const
{
	switch (Mode)
	{
	case EServerMode::Dev:   return DevConfig;
	case EServerMode::Stage: return StageConfig;
	case EServerMode::Live:  return LiveConfig;
	default:                 return DevConfig;
	}
}
