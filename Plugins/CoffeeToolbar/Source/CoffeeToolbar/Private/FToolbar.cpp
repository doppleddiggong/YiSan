// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file FToolbar.cpp
 * @brief Coffee Toolbar 모듈의 라이프사이클과 메뉴 등록 로직을 구현합니다.
 */
#include "FToolbar.h"
#include "Common/FToolbarStyle.h"
#include "Common/FToolbarCommands.h"
#include "Settings/UToolbarSettings.h"

#include "ToolMenus.h"

#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"

#include "Camera/CameraActor.h"
#include "Command/FCommandFeature.h"
#include "Level/FLevelFeature.h"
#include "Screenshot/FScreenshotFeature.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "FDoppleToolbar"

/** @brief 툴바 기능들을 초기화하고 메뉴 확장을 등록합니다. */
void FToolbar::StartupModule()
{
	FToolbarStyle::Initialize();
	FToolbarStyle::ReloadTextures();

	FToolbarCommands::Unregister();

	LevelFeature = MakeUnique<FLevelFeature>();
	ScreenshotFeature = MakeUnique<FScreenshotFeature>();
	CommandFeature = MakeUnique<FCommandFeature>();
	FolderFeature = MakeUnique<FFolderFeature>();
	NetworkTestFeature = MakeUnique<FNetworkTestFeature>();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FToolbar::RegisterMenus));
}

/** @brief 툴바 콘텐츠를 등록 해제하고 기능 인스턴스를 해제합니다. */
void FToolbar::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FToolbarStyle::Shutdown();

	FToolbarCommands::Unregister();
}

/** @brief 레벨 에디터 툴바에 Coffee Toolbar 위젯을 추가합니다. */
void FToolbar::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"); 
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");

	const UToolbarSettings* Settings = GetDefault<UToolbarSettings>();
	if (Settings->bEnableLevelFeature)
	{
		const TAttribute<FText> LabelAttr = TAttribute<FText>::CreateLambda([this]()
		{
			if (LevelFeature->SelectedMapPackage.IsEmpty())
				return NSLOCTEXT("CoffeeToolbar", "Idle", "Level Selector");
			
			const FString AssetName = FPackageName::GetLongPackageAssetName(LevelFeature->SelectedMapPackage);
			return FText::FromString(AssetName);
		});

		FToolMenuEntry DropdownEntry = FToolMenuEntry::InitComboButton(
			"CoffeeToolbarDropdown",
			FUIAction(),
			FOnGetContent::CreateRaw(LevelFeature.Get(), &FLevelFeature::GenerateLevelMenu),
			LabelAttr,
			LOCTEXT("CoffeeToolbarDropdown_Tooltip", "Select Level"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Level")
		);
		DropdownEntry.StyleNameOverride = "CalloutToolbar";
		Section.AddEntry(DropdownEntry);

		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"CoffeeToolbar_Open",
			FUIAction(FExecuteAction::CreateRaw(LevelFeature.Get(), &FLevelFeature::OnSelectedMap_Open)),
			NSLOCTEXT("CoffeeToolbar","Open","Open"),
			NSLOCTEXT("CoffeeToolbar","Open_Tip","Open The Selected Level"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Documentation_16x")
		));

		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"CoffeeToolbar_PlaySelectedViewport",
			FUIAction(FExecuteAction::CreateRaw(LevelFeature.Get(), &FLevelFeature::OnSelectedMap_PlaySelectedViewport)),
			NSLOCTEXT("CoffeeToolbar","PlaySelectedViewport","Play Selected Viewport"),
			NSLOCTEXT("CoffeeToolbar","PlaySelectedViewport_Tip","Play In The Selected Editor Viewport"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Play")
		));

		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"CoffeeToolbar_PlayPIE",
			FUIAction(FExecuteAction::CreateRaw(LevelFeature.Get(), &FLevelFeature::OnSelectedMap_PlayInEditor)),
			NSLOCTEXT("CoffeeToolbar","PlayPIE","Play In Editor"),
			NSLOCTEXT("CoffeeToolbar","PlayPIE_Tip","Selected Editor Play In Editor"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.PadEvent_16x")
		));
	}

    if (Settings->bEnableScreenshotFeature)
    {
        FToolMenuEntry ScreenshotCombo = FToolMenuEntry::InitComboButton(
                "CoffeeToolbar_Screenshot",
                FUIAction(),
                FOnGetContent::CreateRaw(ScreenshotFeature.Get(), &FScreenshotFeature::GenerateScreenshotMenu),
                NSLOCTEXT("CoffeeToolbar", "Screenshot", "Screenshot"),
                NSLOCTEXT("CoffeeToolbar", "Screenshot_Tip", "Capture the active viewport with predefined resolutions."),
                FSlateIconFinder::FindIconForClass(ACameraActor::StaticClass())
        );
        ScreenshotCombo.StyleNameOverride = "CalloutToolbar";
        Section.AddEntry(ScreenshotCombo);
    }

	if (Settings->bEnableCommandFeature)
	{
		FToolMenuEntry CommandsComboButton = FToolMenuEntry::InitComboButton(
			"CommandsComboButton",
			FUIAction(),
			FOnGetContent::CreateRaw(CommandFeature.Get(), &FCommandFeature::GenerateCommandsMenu),
			NSLOCTEXT("CoffeeToolbar", "CommandsMenu", "Commands"),
			NSLOCTEXT("CoffeeToolbar", "CommandsMenu_Tooltip", "Execute custom commands"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings")
		);
		CommandsComboButton.StyleNameOverride = "CalloutToolbar";
		Section.AddEntry(CommandsComboButton);
	}

	if ( Settings->bEnableFolderFeature )
	{
		FToolMenuEntry FolderCombo = FToolMenuEntry::InitComboButton(
			"FolderComboButton",
			FUIAction(),
			FOnGetContent::CreateRaw(FolderFeature.Get(), &FFolderFeature::GenerateFolderMenu),
			NSLOCTEXT("CoffeeToolbar", "FolderMenu", "Folders"),
			NSLOCTEXT("CoffeeToolbar", "FolderMenu_Tooltip", "Open custom project folders"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "SystemWideCommands.FindInContentBrowser")
		);
		FolderCombo.StyleNameOverride = "CalloutToolbar";
		Section.AddEntry(FolderCombo);
	}

	if ( Settings->bEnableNetworkTestFeature )
	{
		FToolMenuEntry NetworkTestCombo = FToolMenuEntry::InitComboButton(
			"NetworkTestComboButton",
			FUIAction(),
			FOnGetContent::CreateRaw(NetworkTestFeature.Get(), &FNetworkTestFeature::GenerateNetworkTestMenu),
			NSLOCTEXT("CoffeeToolbar", "NetworkTestMenu", "Network Tests"),
			NSLOCTEXT("CoffeeToolbar", "NetworkTestMenu_Tooltip", "Execute API network tests"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Download")
		);
		NetworkTestCombo.StyleNameOverride = "CalloutToolbar";
		Section.AddEntry(NetworkTestCombo);
	}

	UToolMenus::Get()->RefreshAllWidgets();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FToolbar, CoffeeToolbar)