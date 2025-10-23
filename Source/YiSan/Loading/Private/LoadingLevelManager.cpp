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
	
	if (HasAuthority())
	{
		UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance());
		if (GI)
		{
			// 게임 인스턴스의 로딩 시퀀스 Step 1을 호출함
			GI->Step1_StartLoadingTargetLevel();
			PRINTLOG(TEXT("로딩 레벨 매니저: Step1 로딩 시작 호출함"));
		}
		else
		{
			PRINTLOG(TEXT("로딩 레벨 매니저: YiSanGameInstance를 찾을 수 없음!"));
		}
	}

	// PRINTLOG(TEXT("로딩중"));

	// if (UYiSanGameInstance* GI = GetGameInstance<UYiSanGameInstance>())
	// {
		
	// 	GI->OnLoadingMapReady();
	// }
	// else
	// {
	// 	PRINTLOG(TEXT("실패"));
	// }
}