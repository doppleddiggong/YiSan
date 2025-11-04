// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file NetworkLog.h
 * @brief YiSan 전반에서 사용하는 공용 인터페이스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"

COFFEELIBRARY_API DECLARE_LOG_CATEGORY_EXTERN(LogCoffeeNetwork, Log, All);

COFFEELIBRARY_API void WriteToGameLog(const FString& Message);

#define NET_CALLINFO FString::Printf(TEXT("[%s:%d] %s"), *FString(__FILE__).Right(FString(__FILE__).Find(TEXT("\\Source\\")) ? FString(__FILE__).Len() - FString(__FILE__).Find(TEXT("\\Source\\")) - 8 : FString(__FILE__).Len()), __LINE__, *FString(__FUNCTION__))

#define NETWORK_LOG(fmt, ...) \
	do { \
		const FString __LogMessage__ = FString::Printf(fmt, ##__VA_ARGS__); \
		const FString __Timestamp__ = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")); \
		const FString __FullMessage__ = FString::Printf(TEXT("[%s] %s : %s"), *__Timestamp__, *NET_CALLINFO, *__LogMessage__); \
		UE_LOG(LogCoffeeNetwork, Warning, TEXT("%s"), *__FullMessage__); \
		WriteToGameLog(__FullMessage__); \
	} while(0)