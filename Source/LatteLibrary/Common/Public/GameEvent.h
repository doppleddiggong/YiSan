/**
 * @file GameEvent.h
 * @brief YiSan 전반에서 사용하는 공용 인터페이스를 선언합니다.
 */
#pragma once

namespace GameEvent
{
	static FName GameStart = FName(TEXT("GameStart"));
	static FName CombatStart = FName(TEXT("CombatStart"));
	static FName CombatResult = FName(TEXT("CombatResult"));
}