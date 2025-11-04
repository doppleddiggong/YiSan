/**
 * @file EDamageType.h
 * @brief EDamageType 클래스를 선언합니다.
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