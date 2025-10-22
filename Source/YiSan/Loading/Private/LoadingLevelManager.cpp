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
	UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance());

	GI->Step2_StartLoadingTargetLevel();
	PRINTLOG(TEXT("실행"));
	
}