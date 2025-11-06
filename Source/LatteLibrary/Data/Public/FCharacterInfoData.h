/**
 * @file FCharacterInfoData.h
 * @brief FCharacterInfoData 구조체를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "Engine/DataTable.h"
#include "FCharacterInfoData.generated.h"

USTRUCT(BlueprintType)
struct FCharacterInfoData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|HP", meta=(ToolTip="캐릭터의 최대 체력입니다."))
	ECharacterType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|HP", meta=(ToolTip="캐릭터의 최대 체력입니다."))
	float HP = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Attack", meta=(ToolTip="콤보 공격의 각 단계별 대미지 값입니다. 배열의 인덱스가 콤보 순서에 해당합니다."))
	float ATK = 50;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Sight", meta=(ToolTip="AI가 적을 인지할 수 있는 최대 거리입니다."))
	float SightLength = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Sight", meta=(ToolTip="AI가 적을 인지할 수 있는 시야각(도)입니다."))
	float SightAngle = 45;	
};
