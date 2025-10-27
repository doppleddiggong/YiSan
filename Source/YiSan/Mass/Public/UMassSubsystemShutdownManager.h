// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "UMyMassSpawnerSubsystem.h"
#include "MassAgentSubsystem.h"
#include "UMyMassAgentComponent.h"
#include "MassEntitySubsystem.h"
#include "UMassSubsystemShutdownManager.generated.h"

/**
 * PIE 종료 시 모든 Mass 시스템 안전 종료
 */
UCLASS()
class YISAN_API UMassSubsystemShutdownManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 모든 Mass Agent 안전 종료 */
	void ShutdownAllMassAgents();

private:
	void ShutdownMassSubsystems();
	void SafeUnregisterAgent(UMassAgentComponent* Agent);

private:
	UMyMassSpawnerSubsystem* SpawnerSubsystem = nullptr;
	UMassAgentSubsystem* AgentSubsystem = nullptr;
	UMassEntitySubsystem* EntitySubsystem = nullptr;
};