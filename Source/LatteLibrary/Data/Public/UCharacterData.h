// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UCharacterData.h
 * @brief UCharacterData 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UCharacterData.generated.h"

UCLASS(BlueprintType)
class LATTELIBRARY_API UCharacterData : public UDataAsset
{
	GENERATED_BODY()

public:
	bool LoadIdleMontage( TSoftObjectPtr<UAnimMontage>& OutMontage) const;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Skeletal)
	TSoftObjectPtr<class USkeletalMesh> MeshData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Skeletal)
	FVector RelativeLocation = FVector(0,0,-88.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Skeletal)
	FRotator RelativeRotator = FRotator(0,0,-90.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Skeletal)
	FVector RelativeScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Skeletal)
	TArray<TSoftObjectPtr<class UMaterialInstanceConstant>> MaterialArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Socket)
	FName LeftHandSocketName = FName("hand_l");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Socket)
	FName RightHandSocketName = FName("hand_r");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Socket)
	FName LeftFootSocketName = FName("foot_l");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Socket)
	FName RightFootSocketName = FName("foot_r");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AnimBlueprint)
	TSoftClassPtr<class UAnimInstance> AnimBluePrint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AnimBlueprint)
	TSoftObjectPtr<class UAnimMontage> IdleAsset;
};
