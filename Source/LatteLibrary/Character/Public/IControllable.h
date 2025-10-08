// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file IControllable.h
 * @brief IControllable 인터페이스 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IControllable.generated.h"

UINTERFACE(MinimalAPI)
class UControllable : public UInterface
{
	GENERATED_BODY()
};

class LATTELIBRARY_API IControllable
{
	GENERATED_BODY()

public:
	virtual void Cmd_Move(const FVector2D& Axis) = 0;		// 이동
	virtual void Cmd_Look(const FVector2D& Axis) = 0;		// 카메라 회전
	virtual void Cmd_Jump() = 0;							// 점프

	virtual void Cmd_Landing() {};							// 랜딩
	
	virtual void Cmd_AltitudeUp() {};						// 고도 상승
	virtual void Cmd_AltitudeDown() {};					// 고도 하강
	virtual void Cmd_AltitudeReleased() {};				// 고도 조절 해제
};
