// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UToolbarSettings.cpp
 * @brief Coffee Toolbar 플러그인의 설정 헬퍼 기능을 구현합니다.
 */

#include "Settings/UToolbarSettings.h"
#include "Settings/FToolbarButtonInfo.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

/** @brief 기본 검색 경로를 초기화하고 저장된 설정을 불러옵니다. */
UToolbarSettings::UToolbarSettings(const FObjectInitializer& ObjectInitializer)	: Super(ObjectInitializer) { }
	
void UToolbarSettings::PostInitProperties()
{
	Super::PostInitProperties();


	ExtraSearchPaths.Empty();
	for (const FString& PathString : ExtraSearchPathsStrings)
	{
		if (!PathString.IsEmpty())
		{
			FDirectoryPath DirPath;
			DirPath.Path = PathString;
			ExtraSearchPaths.Add(DirPath);
		}
	}
}

void UToolbarSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UToolbarSettings, ExtraSearchPathsStrings))
	{
		ExtraSearchPaths.Empty();
		for (const FString& PathString : ExtraSearchPathsStrings)
		{
			if (!PathString.IsEmpty())
			{
				FDirectoryPath DirPath;
				DirPath.Path = PathString;
				ExtraSearchPaths.Add(DirPath);
			}
		}

		// Manual save for ExtraSearchPathsStrings
		FString ConfigFilename = GetDefault<UToolbarSettings>()->GetDefaultConfigFilename();
		const FString ConfigSectionName = TEXT("/Script/CoffeeToolbar.ToolbarSettings"); // Renamed local variable
		const FString PropertyName = TEXT("ExtraSearchPathsStrings");

		// Correct way to save TArray<FString>
		GConfig->SetArray(*ConfigSectionName, *PropertyName, ExtraSearchPathsStrings, *ConfigFilename);

		GConfig->Flush(true, *ConfigFilename); // Force a full flush

		// Persist to project default config via CDO (defaultconfig class)
		if (UToolbarSettings* Defaults = GetMutableDefault<UToolbarSettings>())
		{
			// keep CDO in sync in case editor instance isn't the CDO
			Defaults->ExtraSearchPathsStrings = ExtraSearchPathsStrings;
			Defaults->SaveConfig();
			GConfig->Flush(false, *Defaults->GetDefaultConfigFilename());
		}
	}
	else
	{
		if (UToolbarSettings* Defaults = GetMutableDefault<UToolbarSettings>())
		{
			Defaults->bEnableLevelFeature = bEnableLevelFeature;
			Defaults->ExtraSearchPathsStrings = ExtraSearchPathsStrings;
			Defaults->bEnableScreenshotFeature = bEnableScreenshotFeature;
			Defaults->bEnableCommandFeature = bEnableCommandFeature;
			Defaults->ToolbarButtons = ToolbarButtons;
			Defaults->bEnableFolderFeature = bEnableFolderFeature;
			Defaults->FoldersToOpen = FoldersToOpen;
			Defaults->bEnableNetworkTestFeature = bEnableNetworkTestFeature;
			Defaults->NetworkTestUrl = NetworkTestUrl;
			Defaults->NetworkTests = NetworkTests;
			Defaults->SaveConfig();
			GConfig->Flush(false, *Defaults->GetDefaultConfigFilename());
		}
	}
}

/**
 * @param OutRoot 에셋 레지스트리와 호환되는 정규화된 루트 경로입니다.
 * @return 정규화에 성공하여 OutRoot가 채워졌다면 true를 반환합니다.
 */
static bool NormalizeUserPath(const FString& In, FName& OutRoot)
{
	FString S = In;
	S.ReplaceInline(TEXT("\\"), TEXT("/"));
	S.TrimStartAndEndInline();

	if (S.Contains(TEXT(":")))
		return false;
	
	if (!S.StartsWith(TEXT("/")))
		S = TEXT("/") + S;

	if (S.StartsWith(TEXT("/Content/")))
		S = S.Replace(TEXT("/Content/"), TEXT("/Game/"));

	S.RemoveFromEnd(TEXT("/"));
	OutRoot = FName(*S);
	return true;
}

/**
 * @brief 에셋 레지스트리 검증을 거친 레벨 검색 루트 목록을 반환합니다.
 * @param bFallbackToGame 사용자 경로가 없을 때 /Game을 추가할지 여부입니다.
 */
TArray<FName> UToolbarSettings::GetSearchRoots(const bool bFallbackToGame)
{
	TArray<FName> Paths;
	bool bAdded = false;

	if (const UToolbarSettings* S = GetDefault<UToolbarSettings>()) // This line has been corrected
	{
		for (const FDirectoryPath& Dir : S->ExtraSearchPaths)
		{
			if (Dir.Path.IsEmpty())
				continue;

			FName Root;
			if (NormalizeUserPath(Dir.Path, Root))
			{
				Paths.AddUnique(Root);
				bAdded = true;
			}
		}
	}

	if (!bAdded && bFallbackToGame)
		Paths.Add(FName(TEXT("/Game")));

	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AR = ARM.Get();

	for (int32 i = Paths.Num() - 1; i >= 0; --i)
	{
		if (!AR.PathExists(Paths[i]))
		{
			Paths.RemoveAt(i);
		}
	}

	return Paths;
}
