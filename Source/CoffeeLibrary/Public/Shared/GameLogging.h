// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Engine/Engine.h"
#include "Shared/FLogWriter.h"
#include "Shared/NetworkLog.h"

COFFEELIBRARY_API DECLARE_LOG_CATEGORY_EXTERN(LogCoffee, Log, All);

// Global log writer accessor
inline FLogWriter& GetLogWriter()
{
	return FLogWriter::Get();
}

// Current function/line info
#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))

// Quick debug: location only
#define PRINTINFO() UE_LOG(LogCoffee, Warning, TEXT("%s"), *CALLINFO)

// Log + file output
#define PRINTLOG(fmt, ...) \
do { \
const FString __LogMessage__ = FString::Printf(fmt, ##__VA_ARGS__); \
const FString __FullMessage__ = FString::Printf(TEXT("%s : %s"), *CALLINFO, *__LogMessage__); \
UE_LOG(LogCoffee, Warning, TEXT("%s"), *__FullMessage__); \
GetLogWriter().WriteLog(__FullMessage__); \
} while(0)

// Screen + log output
#define PRINT_STRING(fmt, ...) \
do { \
const FString __msg__ = FString::Printf(fmt, ##__VA_ARGS__); \
const FString __FullMessage__ = FString::Printf(TEXT("%s : %s"), *CALLINFO, *__msg__); \
UE_LOG(LogCoffee, Warning, TEXT("%s"), *__FullMessage__); \
GetLogWriter().WriteLog(__FullMessage__); \
if (GEngine) { \
GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, __msg__); \
} \
} while(0)

