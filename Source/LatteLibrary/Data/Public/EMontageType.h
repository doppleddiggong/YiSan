/**
 * @file EMontageType.h
 * @brief EMontageType 열거형에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

UENUM(BlueprintType)
enum class EMontageType : uint8
{
	None,
	Idle,
	Death,
	Max
};