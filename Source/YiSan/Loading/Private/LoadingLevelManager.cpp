#include "LoadingLevelManager.h"

#include "GameLogging.h"
#include "YiSanGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

ALoadingLevelManager::ALoadingLevelManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALoadingLevelManager::BeginPlay()
{
	Super::BeginPlay();
	
	PRINTLOG(TEXT("로딩중"));

	if (UYiSanGameInstance* GI = GetGameInstance<UYiSanGameInstance>())
	{
		
		GI->OnLoadingMapReady();
	}
	else
	{
		PRINTLOG(TEXT("실패"));
	}
}