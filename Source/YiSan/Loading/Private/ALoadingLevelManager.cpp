#include "ALoadingLevelManager.h"

#include "GameLogging.h" 
#include "YiSanLoading.h"

ALoadingLevelManager::ALoadingLevelManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALoadingLevelManager::BeginPlay()
{
	Super::BeginPlay();
}