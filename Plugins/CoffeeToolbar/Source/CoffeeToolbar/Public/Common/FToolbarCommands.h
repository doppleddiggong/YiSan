// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "Framework/Commands/Commands.h"
#include "Common/FToolbarStyle.h"

class FToolbarCommands : public TCommands<FToolbarCommands>
{
public:
	FToolbarCommands()
		: TCommands<FToolbarCommands>(
			TEXT("CoffeeToolbar"),
			NSLOCTEXT("Contexts", "CoffeeToolbar", "CoffeeToolbar Plugin"),
			NAME_None,
			FToolbarStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
};
