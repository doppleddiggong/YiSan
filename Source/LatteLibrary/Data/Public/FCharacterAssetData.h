/**
 * @file FCharacterAssetData.h
 * @brief FCharacterAssetData 구조체에 대한 Doxygen 주석을 제공합니다.
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