// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UPopup.h"
#include "Engine/World.h"



UUPopup* UUPopup::Get(UWorld* World)
{
	static UUPopup* instance = nullptr;
	if (!instance)
	{
		instance = NewObject<UUPopup>(World, UUPopup::StaticClass());
		// 혹시 GC 걸릴까봐
		instance->AddToRoot();
		UE_LOG(LogTemp, Warning, TEXT("[UPopup]이 인스턴트를 생성하였씁니다"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPopup]이 이미 생성되었습니다 "));
	}
	return instance;
}


void UUPopup::BroadcastBuildingEvent(EBuildingType BuildingType)
{
	
	FString BuildingName = UEnum::GetValueAsString(BuildingType);
	UE_LOG(LogTemp, Warning, TEXT("[UPopup] 빌딩 이름은 : %s"), *BuildingName);

	OnBuildingEvent.Broadcast(BuildingType);
}