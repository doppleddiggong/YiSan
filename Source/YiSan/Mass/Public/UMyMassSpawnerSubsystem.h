// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "MassSpawnerSubsystem.h"
#include "UMyMassSpawnerSubsystem.generated.h"

/**
 * MassSpawnerSubsystem 상속
 * PIE Stop 시 스폰 안전 종료 기능 추가
 */
UCLASS()
class YISAN_API UMyMassSpawnerSubsystem : public UMassSpawnerSubsystem
{
	GENERATED_BODY()

public:
	/** 스폰러 안전 종료 */
	void ShutdownSpawner();
};