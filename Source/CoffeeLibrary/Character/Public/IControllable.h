// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IControllable.generated.h"

UINTERFACE(MinimalAPI)
class UControllable : public UInterface
{
	GENERATED_BODY()
};

class IControllable
{
	GENERATED_BODY()

public:
	virtual void Cmd_Move(const FVector2D& Axis) = 0;		// 이동
	virtual void Cmd_Look(const FVector2D& Axis) = 0;		// 카메라 회전
	virtual void Cmd_Jump() = 0;							// 점프

	virtual void Cmd_Landing() = 0;							// 랜딩
	
	virtual void Cmd_AltitudeUp() = 0;						// 고도 상승
	virtual void Cmd_AltitudeDown() = 0;					// 고도 하강
	virtual void Cmd_AltitudeReleased() = 0;				// 고도 조절 해제
};
