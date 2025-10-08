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
UToolbarSettings::UToolbarSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 기능 활성화 기본값 설정
	bEnableCommandFeature = true;
	bEnableLevelFeature = true;
	bEnableScreenshotFeature = true;

	// 기본 툴바 버튼 목록 설정
	ToolbarButtons.Add(FToolbarButtonInfo{
		FName("StatFPS"), FString("Stat FPS"), FString("Toggle Stat FPS"),
		FString("stat fps"), FName("Icons.Monitor"), true
	});
	ToolbarButtons.Add(FToolbarButtonInfo{
		FName("StatUnit"), FString("Stat Unit"), FString("Toggle Stat Unit"),
		FString("stat unit"), FName("Icons.Monitor"), true
	});
	ToolbarButtons.Add(FToolbarButtonInfo{
		FName("ShowCollision"), FString("Show Collision"), FString("Toggle Show Collision"),
		FString("show collision"), FName("Icons.Collision"), true
	});
	ToolbarButtons.Add(FToolbarButtonInfo{
		FName("ShowDebugAI"), FString("Show Debug AI"), FString("Toggle Show Debug AI"),
		FString("showdebug ai"), FName("Icons.AI"), true
	});
	ToolbarButtons.Add(FToolbarButtonInfo{
		FName("StatSceneRendering"), FString("Stat SceneRendering"), FString("Toggle Stat SceneRendering"),
		FString("stat scenerendering"), FName("Picto/Pictoicon_jewel_3"), true
	});
	
	FDirectoryPath P;
	P.Path = TEXT("Game/CustomContents/Levels");
	ExtraSearchPaths.Add(P);

	ReloadConfig(); // .ini 파일이 존재하면 C++ 기본값을 덮어씁니다.
}

/**
 * @brief 사용자 입력 디렉터리 문자열을 콘텐츠 루트 형식으로 정규화합니다.
 * @param In 사용자 입력 원본 경로 문자열.
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

	if (const UToolbarSettings* S = GetDefault<UToolbarSettings>())
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
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid search path (cannot normalize): %s"), *Dir.Path);
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
			UE_LOG(LogTemp, Warning, TEXT("Skip invalid search path: %s"), *Paths[i].ToString());
			Paths.RemoveAt(i);
		}
	}

	return Paths;
}