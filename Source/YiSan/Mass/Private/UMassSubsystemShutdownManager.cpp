// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#include "UMassSubsystemShutdownManager.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "MassAgentComponent.h"

void UMassSubsystemShutdownManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UWorld* World = GetWorld())
    {
        SpawnerSubsystem = World->GetSubsystem<UMyMassSpawnerSubsystem>();
        AgentSubsystem   = World->GetSubsystem<UMassAgentSubsystem>();
        EntitySubsystem  = World->GetSubsystem<UMassEntitySubsystem>();
    }
}

void UMassSubsystemShutdownManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("[ShutdownManager] Begin Mass shutdown sequence"));
    ShutdownMassSubsystems();
    Super::Deinitialize();
}

void UMassSubsystemShutdownManager::ShutdownMassSubsystems()
{
    if (SpawnerSubsystem)
    {
        SpawnerSubsystem->ShutdownSpawner();
        SpawnerSubsystem = nullptr;
    }

    if (AgentSubsystem)
    {
        AgentSubsystem = nullptr;
    }

    if (EntitySubsystem)
    {
        EntitySubsystem = nullptr;
    }
}

void UMassSubsystemShutdownManager::ShutdownAllMassAgents()
{
    if (!GEngine) return;

    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        if (!World) continue;

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;

            TArray<UMassAgentComponent*> AgentComponents;
            Actor->GetComponents<UMassAgentComponent>(AgentComponents);

            for (UMassAgentComponent* Agent : AgentComponents)
            {
                SafeUnregisterAgent(Agent);
            }
        }
    }
}

void UMassSubsystemShutdownManager::SafeUnregisterAgent(UMassAgentComponent* Agent)
{
    if (!Agent) return;

    // UMyMassAgentComponent로 캐스트 후 상태 확인
    if (UMyMassAgentComponent* MyAgent = Cast<UMyMassAgentComponent>(Agent))
    {
        if (MyAgent->GetAgentState() == EAgentComponentState::PuppetPendingReplication)
        {
            UE_LOG(LogTemp, Warning, TEXT("Skipping Unregister for %s due to PuppetPendingReplication"), *Agent->GetName());
            return;
        }
    }

    Agent->UnregisterWithAgentSubsystem();
}