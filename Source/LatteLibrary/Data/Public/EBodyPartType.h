/**
 * @file EBodyPartType.h
 * @brief EBodyPartType 열거형에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

UENUM(BlueprintType)
enum class EBodyPartType : uint8
{
	None,
	Hand_L 	UMETA(DisplayName = "왼손"),
	Hand_R 	UMETA(DisplayName = "오른손"),
	Foot_L 	UMETA(DisplayName = "왼발"),
	Foot_R 	UMETA(DisplayName = "오른발"),

	Max,
};