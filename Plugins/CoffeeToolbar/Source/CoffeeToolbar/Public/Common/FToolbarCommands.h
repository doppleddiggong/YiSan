// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "Framework/Commands/Commands.h"
#include "Common/FToolbarStyle.h"

/**
 * @file FToolbarCommands.h
 * @brief Coffee Toolbar 플러그인에서 사용하는 명령 바인딩을 선언합니다.
 */

/**
 * @brief 툴바 액션에 사용할 Slate UI 명령 기술자를 정의합니다.
 */
class FToolbarCommands : public TCommands<FToolbarCommands>
{
public:
        /** @brief 지역화 컨텍스트와 스타일 참조를 설정하여 명령 세트를 구성합니다. */
        FToolbarCommands()
                : TCommands<FToolbarCommands>(
                        TEXT("CoffeeToolbar"),
                        NSLOCTEXT("Contexts", "CoffeeToolbar", "CoffeeToolbar Plugin"),
                        NAME_None,
                        FToolbarStyle::GetStyleSetName())
        {
        }

        /** @brief 툴바 액션에 사용할 Slate 명령을 등록합니다. */
        virtual void RegisterCommands() override;

public:
        /** @brief 플러그인의 주요 동작을 실행하는 UI 명령 기술자입니다. */
        TSharedPtr<FUICommandInfo> PluginAction;
};
