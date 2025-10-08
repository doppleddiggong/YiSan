/**
 * @file FScreenshotFeature.cpp
 * @brief Coffee Toolbar의 스크린샷 캡처 헬퍼 기능을 구현합니다.
 */
#include "Screenshot/FScreenshotFeature.h"
#include "Common/FCommon.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "IAssetViewport.h"
#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "Styling/AppStyle.h"

/** @brief 스크린샷 기능의 기본 생성자입니다. */
FScreenshotFeature::FScreenshotFeature()
{
}

/** @brief 스크린샷 옵션을 제공하는 메뉴 위젯을 구성합니다. */
TSharedRef<SWidget> FScreenshotFeature::GenerateScreenshotMenu()
{
    FMenuBuilder MenuBuilder(true, nullptr);

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("CoffeeToolbar", "Screenshot_QuickPreview", "x1"),
        NSLOCTEXT("CoffeeToolbar", "Screenshot_QuickPreview_Tooltip", "현재 활성 뷰포트를 1배 해상도로 빠르게 캡쳐합니다."),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.TakeScreenshot"),
        FUIAction(FExecuteAction::CreateRaw(this, &FScreenshotFeature::CaptureActiveViewport, 1))
    );

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("CoffeeToolbar", "Screenshot_Standard", "x2"),
        NSLOCTEXT("CoffeeToolbar", "Screenshot_Standard_Tooltip", "현재 활성 뷰포트를 2배 배율로 캡쳐합니다."),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.TakeScreenshot"),
        FUIAction(FExecuteAction::CreateRaw(this, &FScreenshotFeature::CaptureActiveViewport, 2))
    );

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("CoffeeToolbar", "Screenshot_High", "x4"),
        NSLOCTEXT("CoffeeToolbar", "Screenshot_High_Tooltip", "현재 활성 뷰포트를 4배 배율로 캡쳐합니다."),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.TakeScreenshot"),
        FUIAction(FExecuteAction::CreateRaw(this, &FScreenshotFeature::CaptureActiveViewport, 4))
    );

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("CoffeeToolbar", "Screenshot_Ultra", "x8"),
        NSLOCTEXT("CoffeeToolbar", "Screenshot_Ultra_Tooltip", "현재 활성 뷰포트를 8배 배율로 캡쳐합니다."),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.TakeScreenshot"),
        FUIAction(FExecuteAction::CreateRaw(this, &FScreenshotFeature::CaptureActiveViewport, 8))
    );

    MenuBuilder.AddMenuSeparator();

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("CoffeeToolbar", "Screenshot_OpenFolder", "캡쳐 폴더 열기"),
        NSLOCTEXT("CoffeeToolbar", "Screenshot_OpenFolder_Tooltip", "스크린샷이 저장된 캡쳐 폴더를 엽니다."),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.TakeScreenshot"),
        FUIAction(FExecuteAction::CreateRaw(this, &FScreenshotFeature::OnOpenScreenShotDir))
    );

    return MenuBuilder.MakeWidget();
}

/** @brief 지정된 배율로 활성 뷰포트에 고해상도 캡처를 요청합니다. */
void FScreenshotFeature::CaptureActiveViewport(int32 ResolutionMultiplier)
{
    if (ResolutionMultiplier <= 0)
    {
        ResolutionMultiplier = 1;
    }

    if (IConsoleVariable* ResolutionCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.HighResScreenshot.ResolutionMultiplier")))
    {
        ResolutionCVar->Set(ResolutionMultiplier);
    }

    if (GEngine && GEditor &&
        GEditor->PlayWorld &&
        GEngine->GameViewport &&
        GEngine->GameViewport->Viewport)
    {
        FScreenshotRequest::RequestScreenshot(true);
        GEngine->GameViewport->Viewport->TakeHighResScreenShot();
        return;
    }

    if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
    {
        if (TSharedPtr<IAssetViewport> AssetViewport = LevelEditorModule->GetFirstActiveViewport())
        {
            FEditorViewportClient& ViewportClient = AssetViewport->GetAssetViewportClient();
            ViewportClient.TakeHighResScreenShot();
            return;
        }
    }

    if (UWorld* World = FCommon::GetActiveTargetWorld())
    {
        const FString Command = FString::Printf(TEXT("HighResShot %d"), ResolutionMultiplier);
        GEditor->Exec(World, *Command);
    }
}

/** @brief 촬영된 스크린샷이 저장된 디렉터리를 엽니다. */
void FScreenshotFeature::OnOpenScreenShotDir()
{
    FString AbsoluteDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ScreenShotDir());
    IFileManager::Get().MakeDirectory(*AbsoluteDir, true);
    FPlatformProcess::ExploreFolder(*AbsoluteDir);
}