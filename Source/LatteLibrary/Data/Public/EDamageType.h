/**
 * @file EDamageType.h
 * @brief EDamageType 열거형에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	None,
	
	Small,
	Normal,
	Large,
	Huge,

	Max
};