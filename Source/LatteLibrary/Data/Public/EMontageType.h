/**
 * @file EMontageType.h
 * @brief EMontageType 클래스를 선언합니다.
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