#include "LoadingLevelManager.h"

#include "GameLogging.h"
#include "YiSanGameInstance.h"

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