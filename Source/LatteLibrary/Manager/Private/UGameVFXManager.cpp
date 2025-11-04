// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UGameVFXManager.cpp
 * @brief UGameVFXManager의 동작을 구현합니다.
 */
#include "UGameVFXManager.h"

#include "EDamageType.h"
#include "GameLogging.h"
#include "UVFXDataAsset.h"
#include "NiagaraFunctionLibrary.h"
#include "FComponentHelper.h"

#define VFX_DATA_PATH TEXT("/Game/CustomContents/MasterData/VFX_Data.VFX_Data")

UGameVFXManager::UGameVFXManager()
{
	if (auto LoadedAsset = FComponentHelper::LoadAsset<UVFXDataAsset>(VFX_DATA_PATH))
	{
		VFXAsset = LoadedAsset;
		VFXDataMap = VFXAsset->VFXData;
	}
}

void UGameVFXManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!VFXAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load VFXDataAsset from path"));
	}
}


bool UGameVFXManager::GetVFXData(EVFXType Type, FVFXData& Out) const
{
	if (const FVFXData* Found = VFXDataMap.Find(Type))
	{
		Out = *Found;
		return true;
	}

	PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
	return false;
}

void UGameVFXManager::ShowVFX( EVFXType Type,
	FVector Location, FRotator Rotator, FVector Scale)
{
	FVFXData Params;
	if (!this->GetVFXData(Type, Params))
		return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		Params.VFXAsset,
		Location + Params.LocationOffset,
		Rotator + Params.RotatorOffset,
		Scale * Params.Scale,
		true,
		true,
		ENCPoolMethod::None,
		true
	);
}
