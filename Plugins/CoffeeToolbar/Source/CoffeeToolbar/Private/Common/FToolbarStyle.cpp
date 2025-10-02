// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file FToolbarStyle.cpp
 * @brief Coffee Toolbar 플러그인의 Slate 스타일 등록을 구현합니다.
 */
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

/** @brief Coffee Toolbar에 등록된 Slate 스타일을 반환합니다. */
const ISlateStyle& FToolbarStyle::Get()
{
	return *Instance;
}

/** @brief Coffee Toolbar 스타일 세트를 Slate 스타일 레지스트리에 등록합니다. */
void FToolbarStyle::Initialize()
{
	if (!Instance.IsValid())
	{
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

/** @brief Coffee Toolbar 스타일 세트를 등록 해제하고 정리합니다. */
void FToolbarStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	ensure(Instance.IsUnique());
	Instance.Reset();
}

/** @brief Coffee Toolbar 스타일 세트의 이름 식별자를 제공합니다. */
FName FToolbarStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("FCoffeeToolbarStyle"));
	return StyleSetName;
}

/** @brief 스타일 세트 인스턴스를 생성하고 사용자 정의 툴바 아이콘을 등록합니다. */
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

/** @brief Slate가 초기화된 경우 Slate 텍스처 리소스를 다시 로드합니다. */
void FToolbarStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}