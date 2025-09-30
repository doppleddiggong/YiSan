#include "Level/FLevelFeature.h"
#include "Settings/UToolbarSettings.h" // For UCoffeeToolbarSettings::GetSearchRoots
#include "Common/FCommon.h" // For GetActiveTargetWorld
#include "ToolMenus.h" // For FMenuBuilder
#include "UnrealEdGlobals.h"
#include "FileHelpers.h"
#include "LevelEditor.h"
#include "IAssetViewport.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/MessageDialog.h"
#include "Editor/UnrealEdEngine.h"
#include "Modules/ModuleManager.h"

FLevelFeature::FLevelFeature()
{
}

void FLevelFeature::EnsureDefaultSelection()
{
    if (!SelectedMapPackage.IsEmpty())
        return;

    TArray<FAssetData> Worlds;
    if (!CollectWorlds(Worlds) || Worlds.Num() == 0)
        return;

    SelectedMapPackage = Worlds[0].PackageName.ToString();
}

bool FLevelFeature::CollectWorlds(TArray<FAssetData>& OutWorlds) const
{
    const TArray<FName> Roots = UToolbarSettings::GetSearchRoots();

    FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AR = ARM.Get();

#if WITH_EDITOR
    // 에셋 레지스트리 스캔이 끝날 때까지 대기(필요 시)
    AR.WaitForCompletion();
#endif

    FARFilter Filter;
    Filter.bRecursivePaths = true;
    Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
    for (const FName& Root : Roots)
        Filter.PackagePaths.Add(Root);

    AR.GetAssets(Filter, OutWorlds);

    OutWorlds.Sort([](const FAssetData& A, const FAssetData& B)
    {
        if (A.AssetName != B.AssetName)
            return A.AssetName.LexicalLess(B.AssetName);
        
        return A.PackageName.LexicalLess(B.PackageName);
    });

    return OutWorlds.Num() > 0;
}

bool FLevelFeature::EnsureValidSelectedMap() const
{
    if (SelectedMapPackage.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No Level Selected!"));
        return false;
    }
    if (!FPackageName::IsValidLongPackageName(SelectedMapPackage))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid long package name: %s"), *SelectedMapPackage);
        return false;
    }
    if (!FPackageName::DoesPackageExist(SelectedMapPackage))
    {
        UE_LOG(LogTemp, Warning, TEXT("Map does not exist: %s"), *SelectedMapPackage);
        return false;
    }
    return true;
}

bool FLevelFeature::LoadSelectedMap() const
{
    if (!EnsureValidSelectedMap())
        return false;

    // 필요 시 저장 유도:
    FEditorFileUtils::SaveDirtyPackages(true, true, false, false);

    if (!UEditorLoadingAndSavingUtils::LoadMap(SelectedMapPackage))
    {
        UE_LOG(LogTemp, Warning, TEXT("Load failed: %s"), *SelectedMapPackage);
        return false;
    }
    return true;
}

TSharedRef<SWidget> FLevelFeature::GenerateLevelMenu()
{
    const TArray<FName> Roots = UToolbarSettings::GetSearchRoots();

    FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AR = ARM.Get();
    
    FARFilter Filter;
    Filter.bRecursivePaths = true;
    Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
    
    for (const FName& Root : Roots)
        Filter.PackagePaths.Add(Root);

    TArray<FAssetData> Worlds;
    AR.GetAssets(Filter, Worlds);

    TMap<FName, int32> NameCount;
    for (const FAssetData& AD : Worlds)
        NameCount.FindOrAdd(AD.AssetName)++;

    Worlds.Sort([](const FAssetData& A, const FAssetData& B)
    {
        if (A.AssetName != B.AssetName)
            return A.AssetName.LexicalLess(B.AssetName);
        
        return A.PackageName.LexicalLess(B.PackageName);
    });

    FMenuBuilder MenuBuilder(true, nullptr);

    for (const FAssetData& AssetData : Worlds)
    {
        const FString PackagePath = AssetData.PackageName.ToString();                    // "/Game/Level/DefaultMap"
        const FString NameOnly    = AssetData.AssetName.ToString();                      // "DefaultMap"
        const FString DirOnly     = FPackageName::GetLongPackagePath(PackagePath);         // "/Game/Level"
        const bool   bDup         = NameCount[AssetData.AssetName] > 1;

        const FText Label   = bDup
            ? FText::FromString(FString::Printf(TEXT("%s  (%s)"), *NameOnly, *DirOnly))
            : FText::FromString(NameOnly);
        const FText Tooltip = FText::FromString(PackagePath);

        MenuBuilder.AddMenuEntry(
            Label, Tooltip, FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &FLevelFeature::OnSelectedMap, PackagePath))
        );
    }

    return MenuBuilder.MakeWidget();
}

void FLevelFeature::OnSelectedMap(FString InLongPackageName)
{
    this->SelectedMapPackage = MoveTemp(InLongPackageName);
    UToolMenus::Get()->RefreshAllWidgets();
    
    UE_LOG(LogTemp, Log, TEXT("Selected Level: %s"), *InLongPackageName);
}

void FLevelFeature::OnSelectedMap_Open()
{
    if (LoadSelectedMap())
        UE_LOG(LogTemp, Log, TEXT("Opened: %s"), *SelectedMapPackage);
}

void FLevelFeature::OnSelectedMap_PlaySelectedViewport()
{
    if (!LoadSelectedMap())
        return;

    TSharedPtr<IAssetViewport> ActiveLevelViewport;
    if (FModuleManager::Get().IsModuleLoaded(TEXT("LevelEditor")))
    {
        auto& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
        ActiveLevelViewport = LevelEditorModule.GetFirstActiveViewport();
    }

    if (GUnrealEd)
    {
        FRequestPlaySessionParams Params;
        Params.WorldType = EPlaySessionWorldType::PlayInEditor;
        Params.SessionDestination = EPlaySessionDestinationType::InProcess;
        if (ActiveLevelViewport.IsValid())
            Params.DestinationSlateViewport = ActiveLevelViewport;
        GUnrealEd->RequestPlaySession(Params);
    }
}

void FLevelFeature::OnSelectedMap_PlayInEditor()
{
    if (!EnsureValidSelectedMap())
        return;

    if (GUnrealEd)
    {
        FRequestPlaySessionParams Params;
        Params.WorldType = EPlaySessionWorldType::PlayInEditor;
        Params.SessionDestination = EPlaySessionDestinationType::InProcess;
        Params.GlobalMapOverride = SelectedMapPackage;
        GUnrealEd->RequestPlaySession(Params);
    }
}