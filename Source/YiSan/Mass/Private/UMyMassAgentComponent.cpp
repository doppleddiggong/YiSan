// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UMyMassAgentComponent.h"
#include "Engine/Engine.h"

void UMyMassAgentComponent::UnregisterWithAgentSubsystem()
{
	// PIE Stop 시 PendingReplication이면 skip
	if (State == EAgentComponentState::PuppetPendingReplication)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyMassAgentComponent::UnregisterWithAgentSubsystem skipped for %s due to PuppetPendingReplication"), *GetName());
		return;
	}

	// 부모 호출
	Super::UnregisterWithAgentSubsystem();
}