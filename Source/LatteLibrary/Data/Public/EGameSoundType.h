/**
 * @file EGameSoundType.h
 * @brief EGameSoundType 열거형에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

UENUM(BlueprintType)
enum class EGameSoundType : uint8
{
	Click,
	Diaglog,
	Disolve,

	Cmd_Approach,
	Enter_Game,
	Door_Open,
};
