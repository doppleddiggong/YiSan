#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Framework/Commands/UIAction.h"
#include "AssetRegistry/AssetData.h" // For FAssetData

class FLevelFeature
{
public:
    FLevelFeature();

    TSharedRef<SWidget> GenerateLevelMenu();
    void OnSelectedMap(FString InLongPackageName);
    void OnSelectedMap_Open();
    void OnSelectedMap_PlaySelectedViewport();
    void OnSelectedMap_PlayInEditor();

    void EnsureDefaultSelection();

private:
    bool CollectWorlds(TArray<FAssetData>& OutWorlds) const;
    bool EnsureValidSelectedMap() const;
    bool LoadSelectedMap() const;

    public:
    FString SelectedMapPackage;
};