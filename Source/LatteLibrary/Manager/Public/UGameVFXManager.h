// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UGameVFXManager.h
 * @brief UGameVFXManager 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "EVFXType.h"
#include "UVFXDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGameVFXManager.generated.h"

/**
 * @brief 게임 내 시각 효과(VFX) 재생을 관리하는 중앙 VFX 관리 서브시스템입니다.
 * @details 데이터 에셋에 미리 정의된 VFX 데이터를 기반으로 특정 위치, 회전, 크기로 파티클 시스템을 스폰하는 기능을 제공합니다.
 */
UCLASS()
class LATTELIBRARY_API UGameVFXManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UGameVFXManager);

    UGameVFXManager();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION(BlueprintCallable, Category="VFX")
    bool GetVFXData(EVFXType Type, FVFXData& Out) const;

	UFUNCTION(BlueprintCallable, Category="VFX")
    void ShowVFX( EVFXType Type, FVector Location, FRotator Rotator, FVector Scale);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UVFXDataAsset> VFXAsset;
	
	UPROPERTY(EditAnywhere, Category = "VFX")
	TMap<EVFXType, FVFXData> VFXDataMap; 
};
