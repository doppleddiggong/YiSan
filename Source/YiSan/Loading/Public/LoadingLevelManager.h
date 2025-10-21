#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LoadingLevelManager.generated.h"

UCLASS()
class YISAN_API ALoadingLevelManager : public AActor
{
	GENERATED_BODY()

public:
	ALoadingLevelManager();

	virtual void BeginPlay() override;
};