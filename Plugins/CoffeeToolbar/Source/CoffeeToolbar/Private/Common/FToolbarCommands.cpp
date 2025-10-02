// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file FToolbarCommands.cpp
 * @brief Coffee Toolbar 플러그인의 Slate 명령 정의를 구현합니다.
 */
#include "Common/FToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FCoffeeToolbarModule"

/** @brief Coffee Toolbar 플러그인에서 사용할 Slate 명령을 등록합니다. */
void FToolbarCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "CoffeeToolbar", "Execute CoffeeToolbar action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
