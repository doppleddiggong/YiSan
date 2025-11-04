/**
 * @file EGameSoundType.h
 * @brief EGameSoundType 클래스를 선언합니다.
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
