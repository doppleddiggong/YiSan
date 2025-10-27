// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "MassAgentComponent.h"
#include "UMyMassAgentComponent.generated.h"

/**
 * 기존 MassAgentComponent를 상속
 * PIE 종료 시 안전하게 상태 확인 후 Unregister 가능하도록 구현
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YISAN_API UMyMassAgentComponent : public UMassAgentComponent
{
	GENERATED_BODY()

public:
	/** 상태 확인용 Getter */
	EAgentComponentState GetAgentState() const { return State; }

	/** UnregisterWithAgentSubsystem 재정의 */
	void UnregisterWithAgentSubsystem();
};