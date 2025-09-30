// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UToolbarSettings.generated.h"

UCLASS(Config=ToolbarSettings)
class COFFEETOOLBAR_API UToolbarSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UToolbarSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FName GetSectionName()  const override { return TEXT("Level Selector"); }

	static TArray<FName> GetSearchRoots(const bool bFallbackToGame = true);
	
	UPROPERTY(EditAnywhere, Config, Category="Search")
	TArray<FDirectoryPath> ExtraSearchPaths;

	UPROPERTY(EditAnywhere, Config, Category="Toolbar")
	TArray<FToolbarButtonInfo> ToolbarButtons;


	UPROPERTY(EditAnywhere, Config, Category="Features")
	bool bEnableLevelFeature = true;
	UPROPERTY(EditAnywhere, Config, Category="Features")
	bool bEnableScreenshotFeature = true;
	UPROPERTY(EditAnywhere, Config, Category="Features")
	bool bEnableCommandFeature = true;
};

	