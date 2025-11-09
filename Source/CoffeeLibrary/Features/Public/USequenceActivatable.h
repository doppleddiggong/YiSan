// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file USequenceActivatable.h
 * @brief USequenceActivatable 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "USequenceActivatable.generated.h"

/**
 * @brief 시퀀스 매니저에 의해 순차적으로 활성화/비활성화될 수 있는 액터 또는 컴포넌트를 위한 인터페이스입니다.
 * @details 이 인터페이스를 구현하는 객체는 Activate 및 Deactivate 이벤트를 수신하여 정해진 순서에 따라 특정 동작을 수행할 수 있습니다.
 */
UINTERFACE(BlueprintType)
class COFFEELIBRARY_API USequenceActivatable : public UInterface
{
	GENERATED_BODY()
};

class COFFEELIBRARY_API ISequenceActivatable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Sequence")
	void Activate(int Index, int TotalIndex, float Duration);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Sequence")
	void Deactivate(int Index, int TotalIndex, float Duration);
};