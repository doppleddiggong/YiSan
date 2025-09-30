#include "Screenshot/FScreenshotFeature.h"
#include "Common/FCommon.h" // For GetActiveTargetWorld
#include "LevelEditor.h" // For FLevelEditorModule
#include "IAssetViewport.h" // For IAssetViewport
#include "Engine/Engine.h" // For GEngine
#include "UnrealEdGlobals.h" // For GEditor
#include "Misc/Paths.h" // For FPaths
#include "HAL/FileManager.h" // For IFileManager
#include "HAL/PlatformProcess.h" // For FPlatformProcess

FScreenshotFeature::FScreenshotFeature()
{
}

void FScreenshotFeature::OnCaptureScreenshot()
{
    // 1) PIE 중이면 게임뷰포트에 고해상도 캡쳐
    if (GEngine && GEditor &&
        GEditor->PlayWorld &&
        GEngine->GameViewport &&
        GEngine->GameViewport->Viewport)
    {
        FScreenshotRequest::RequestScreenshot(true);
        GEngine->GameViewport->Viewport->TakeHighResScreenShot();
        return;
    }

    // 2) 에디터 레벨 뷰포트가 있으면 그쪽에서 캡쳐
    if (FLevelEditorModule* LEM = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
    {
        if (TSharedPtr<IAssetViewport> AV = LEM->GetFirstActiveViewport())
        {
            FEditorViewportClient& VC = AV->GetAssetViewportClient();
            VC.TakeHighResScreenShot();
            return;
        }
    }

    if (UWorld* W = FCommon::GetActiveTargetWorld())
        GEditor->Exec(W, TEXT("HighResShot 1"));
}

void FScreenshotFeature::OnOpenScreenShotDir()
{
    FString Dir = FPaths::ScreenShotDir();
    FString AbsoluteDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Dir);
    IFileManager::Get().MakeDirectory(*AbsoluteDir, true);
    FPlatformProcess::ExploreFolder(*AbsoluteDir);
}