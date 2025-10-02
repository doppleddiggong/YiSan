/**
 * @file EVFXType.h
 * @brief EVFXType 열거형에 대한 Doxygen 주석을 제공합니다.
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