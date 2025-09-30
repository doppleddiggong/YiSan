// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "Modules/ModuleManager.h"

#include "Command/FCommandFeature.h"
#include "Level/FLevelFeature.h"
#include "Screenshot/FScreenshotFeature.h"

class FToolbar : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	
private:
	void RegisterMenus();

	TUniquePtr<FCommandFeature> CommandFeature;
	TUniquePtr<FLevelFeature> LevelFeature;
	TUniquePtr<FScreenshotFeature> ScreenshotFeature;
};