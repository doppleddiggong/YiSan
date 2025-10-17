// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UPopup.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UUPopup* UUPopup::Get(UWorld* World)
{
	static TWeakObjectPtr<UUPopup> Instance;

	if (!Instance.IsValid())
	{
		UUPopup* NewInstance = NewObject<UUPopup>(GetTransientPackage(), UUPopup::StaticClass());
		Instance = NewInstance;
		UE_LOG(LogTemp, Warning, TEXT("[UPopup] 새 인스턴스가 생성되었습니다."));
	}

	return Instance.Get();
}

void UUPopup::BroadcastBuildingEvent(EBuildingType BuildingType)
{
	
	FString BuildingName = UEnum::GetValueAsString(BuildingType);
	UE_LOG(LogTemp, Warning, TEXT("[UPopup] 빌딩 이름은 : %s"), *BuildingName);

	OnBuildingEvent.Broadcast(BuildingType);
}