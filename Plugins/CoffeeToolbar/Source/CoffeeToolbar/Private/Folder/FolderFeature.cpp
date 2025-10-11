#include "Folder/FolderFeature.h"
#include "Settings/UToolbarSettings.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FFolderFeature"

DEFINE_LOG_CATEGORY_STATIC(LogCoffeeToolbarFolderFeature, Log, All);

TSharedRef<SWidget> FFolderFeature::GenerateFolderMenu()
{
    FMenuBuilder MenuBuilder(true, nullptr);
    const UToolbarSettings* Settings = GetDefault<UToolbarSettings>();

    if (Settings && !Settings->FoldersToOpen.IsEmpty())
    {
        for (const FFolderPathInfo& FolderInfo : Settings->FoldersToOpen)
        {
            if (FolderInfo.Path.IsEmpty())
            {
                continue;
            }

            FUIAction Action(FExecuteAction::CreateRaw(this, &FFolderFeature::OpenFolder, FolderInfo.Path));
            
            MenuBuilder.AddMenuEntry(
                FText::FromString(FolderInfo.Label.IsEmpty() ? FolderInfo.Path : FolderInfo.Label),
                FText::FromString(FString::Printf(TEXT("Open %s"), *FolderInfo.Path)),
                FSlateIcon(),
                Action
            );
        }
    }
    else
    {
        MenuBuilder.AddMenuEntry(
            LOCTEXT("NoFoldersConfigured", "No folders configured"),
            LOCTEXT("NoFoldersConfigured_Tooltip", "Add folder paths in Project Settings > Plugins > Level Selector"),
            FSlateIcon(),
            FUIAction()
        );
    }

    return MenuBuilder.MakeWidget();
}

static FString ResolveProjectRelative(const FString& InPath)
{
    FString P = InPath; 
    FPaths::NormalizeFilename(P);
    P.TrimStartAndEndInline();

    if (!FPaths::IsRelative(P))
        return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*P);

    const FString Full = FPaths::Combine(FPaths::ProjectDir(), P);
    return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Full);
}

void FFolderFeature::OpenFolder(FString Path)
{
    const FString FullPath = ResolveProjectRelative(Path);
    FString FolderToOpen = FullPath;

    if (!FPaths::DirectoryExists(FolderToOpen) && FPaths::FileExists(FolderToOpen))
        FolderToOpen = FPaths::GetPath(FolderToOpen);

    if (!FPaths::DirectoryExists(FolderToOpen))
        FPlatformProcess::ExploreFolder(*FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()));
    else
        FPlatformProcess::ExploreFolder(*FolderToOpen);
}
#undef LOCTEXT_NAMESPACE
