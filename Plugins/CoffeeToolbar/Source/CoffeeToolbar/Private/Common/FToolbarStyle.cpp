// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "Common/FToolbarStyle.h"
#include "Settings/UToolbarSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedPtr<FSlateStyleSet> FToolbarStyle::Instance = nullptr;

const ISlateStyle& FToolbarStyle::Get()
{
	return *Instance;
}

void FToolbarStyle::Initialize()
{
	if (!Instance.IsValid())
	{
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FToolbarStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	ensure(Instance.IsUnique());
	Instance.Reset();
}

FName FToolbarStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("FCoffeeToolbarStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FToolbarStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ToolbarStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("CoffeeToolbar")->GetBaseDir() / TEXT("Resources"));

	const UToolbarSettings* Settings = GetDefault<UToolbarSettings>();
	for (const auto& Button : Settings->ToolbarButtons)
	{
		if (!Button.Icon.IsNone() && !Button.Icon.ToString().Contains(TEXT(".")))
		{
			FString IconPath = Button.Icon.ToString();
			Style->Set(Button.Icon, new IMAGE_BRUSH(IconPath, Icon20x20));
		}
	}

	Style->Set("CoffeeToolbar.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FToolbarStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}