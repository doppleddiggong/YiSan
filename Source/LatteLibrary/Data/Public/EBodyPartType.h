/**
 * @file EBodyPartType.h
 * @brief EBodyPartType 클래스를 선언합니다.
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