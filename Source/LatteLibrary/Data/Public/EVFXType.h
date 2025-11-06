/**
 * @file EVFXType.h
 * @brief EVFXType 클래스를 선언합니다.
 */
#pragma once

UENUM(BlueprintType)
enum class EVFXType : uint8
{
	None,
	Hit,
	Explosion,
	Max,
};