// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UTweenAnimInstance.cpp
 * @brief UTweenAnimInstance의 동작을 구현합니다.
 */
#include "UTweenAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

/** @brief 알파를 0~1 범위에서 증가 또는 감소시키며 트윈 상태를 업데이트한다. */
void UTweenAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	const float AlphaStep  = DeltaSeconds / TweenDuration;
	const float Dir        = bActivate ? 1.f : -1.f;

	TweenAlpha = FMath::Clamp(TweenAlpha + AlphaStep * Dir, 0.f, 1.f);
}

/** @brief Ease 커브가 유효하면 값을 보간하여 반환한다. */
float UTweenAnimInstance::GetEasedAlpha() const
{
	if (EaseCurve)
		return EaseCurve->GetFloatValue(TweenAlpha);
	return TweenAlpha;
}