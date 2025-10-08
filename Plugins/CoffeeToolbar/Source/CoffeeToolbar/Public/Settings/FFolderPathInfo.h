#pragma once

#include "CoreMinimal.h"
#include "FFolderPathInfo.generated.h"

USTRUCT(BlueprintType)
struct FFolderPathInfo
{
    GENERATED_BODY()

    /** 드롭다운 메뉴에 표시될 폴더의 이름입니다. */
    UPROPERTY(EditAnywhere, Category="Folder Info")
    FString Label;

    /** 프로젝트 루트 폴더 기준의 상대 경로입니다. (예: "Saved", "Documents/DevLog") */
    UPROPERTY(EditAnywhere, Category="Folder Info")
    FString Path;
};