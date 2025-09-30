// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

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

void FToolbar::StartupModule()
{
	FToolbarStyle::Initialize();
	FToolbarStyle::ReloadTextures();

	FToolbarCommands::Unregister();

	LevelFeature = MakeUnique<FLevelFeature>();
	ScreenshotFeature = MakeUnique<FScreenshotFeature>();
	CommandFeature = MakeUnique<FCommandFeature>();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FToolbar::RegisterMenus));
}

void FToolbar::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FToolbarStyle::Shutdown();

	FToolbarCommands::Unregister();
}

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
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"CoffeeToolbar_Screenshot",
			FUIAction(FExecuteAction::CreateRaw(ScreenshotFeature.Get(), &FScreenshotFeature::OnCaptureScreenshot)),
			NSLOCTEXT("CoffeeToolbar", "Screenshot", "Screenshot"),
			NSLOCTEXT("CoffeeToolbar", "Screenshot_Tip", "Take a screenshot of the active viewport (PIE or Editor)"),
			FSlateIconFinder::FindIconForClass(ACameraActor::StaticClass()) // ← 카메라 아이콘
		));

		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"DoppleToolbar_OpenShotDir",
			FUIAction(FExecuteAction::CreateRaw(ScreenshotFeature.Get(), &FScreenshotFeature::OnOpenScreenShotDir)),
			NSLOCTEXT("CoffeeToolbar", "OpenShotDir", "Open Screenshot Folder"),
			NSLOCTEXT("CoffeeToolbar", "OpenShotDir_Tip", "Open the project\'s screenshot directory"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.FolderOpen")
		));
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

	UToolMenus::Get()->RefreshAllWidgets();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FToolbar, CoffeeToolbar)