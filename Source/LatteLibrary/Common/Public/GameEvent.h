/**
 * @file GameEvent.h
 * @brief GameEvent 네임스페이스에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

namespace GameEvent
{
	static FName GameStart = FName(TEXT("GameStart"));
	static FName CombatStart = FName(TEXT("CombatStart"));
	static FName CombatResult = FName(TEXT("CombatResult"));
}