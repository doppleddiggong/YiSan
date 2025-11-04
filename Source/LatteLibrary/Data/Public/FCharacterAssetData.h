/**
 * @file FCharacterAssetData.h
 * @brief UCharacterData 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "Engine/DataTable.h"
#include "FCharacterAssetData.generated.h"

USTRUCT(BlueprintType)
struct FCharacterAssetData: public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
	ECharacterType CharacterType = ECharacterType::Type01;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
	TSoftObjectPtr<class UCharacterData> CharacterDataAsset;
};