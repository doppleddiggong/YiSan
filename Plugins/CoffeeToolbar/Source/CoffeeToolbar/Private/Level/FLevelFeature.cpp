/**
 * @file FLevelFeature.cpp
 * @brief Coffee Toolbar에서 레벨 선택을 돕는 헬퍼 기능을 구현합니다.
 */
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

/** @brief 레벨 선택을 위한 기본 상태를 초기화합니다. */
FLevelFeature::FLevelFeature()
{
}

/** @brief 툴바 초기화 시 유효한 레벨이 선택되도록 보장합니다. */
void FLevelFeature::EnsureDefaultSelection()
{
    if (!SelectedMapPackage.IsEmpty())
        return;

    TArray<FAssetData> Worlds;
    if (!CollectWorlds(Worlds) || Worlds.Num() == 0)
        return;

    SelectedMapPackage = Worlds[0].PackageName.ToString();
}

/** @brief 설정된 검색 경로를 사용해 월드 에셋 목록을 수집합니다. */
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

/** @brief 현재 선택된 레벨이 존재하는 패키지를 가리키는지 검증합니다. */
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

/** @brief 더티 패키지 저장을 유도한 뒤 선택된 맵을 에디터에 로드합니다. */
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

/** @brief 사용 가능한 레벨 에셋을 나열하는 메뉴를 구성합니다. */
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

/** @brief 메뉴에서 새로운 맵을 선택했을 때의 처리를 담당합니다. */
void FLevelFeature::OnSelectedMap(FString InLongPackageName)
{
    this->SelectedMapPackage = MoveTemp(InLongPackageName);
    UToolMenus::Get()->RefreshAllWidgets();
    
    UE_LOG(LogTemp, Log, TEXT("Selected Level: %s"), *InLongPackageName);
}

/** @brief 선택된 맵 에셋을 에디터에서 엽니다. */
void FLevelFeature::OnSelectedMap_Open()
{
    if (LoadSelectedMap())
        UE_LOG(LogTemp, Log, TEXT("Opened: %s"), *SelectedMapPackage);
}

/** @brief 현재 활성화된 뷰포트에서 선택된 레벨을 실행합니다. */
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

/** @brief 선택된 맵으로 새로운 뷰포트에서 PIE를 실행합니다. */
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