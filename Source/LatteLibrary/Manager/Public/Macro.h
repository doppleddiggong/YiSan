// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file Macro.h
 * @brief Manager 모듈에서 공통으로 사용하는 매크로 정의에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once


// 공통 선언-구현 매크로
#define DEFINE_SUBSYSTEM_GETTER_INLINE(ClassName) \
UFUNCTION(BlueprintPure, Category="CoffeeLibrary|Subsystem", meta=(WorldContext="WorldContextObject")) \
static ClassName* Get(UObject* WorldContext) \
{ \
if (!WorldContext) \
return nullptr; \
if (auto* GI = WorldContext->GetWorld()->GetGameInstance()) \
return GI->GetSubsystem<ClassName>(); \
return nullptr; \
}

// 공통 선언 매크로
#define DECLARE_SUBSYSTEM_GETTER(ClassName) \
UFUNCTION(BlueprintPure, Category="CoffeeLibrary|Subsystem", meta=(WorldContext="WorldContextObject")) \
static ClassName* Get(UObject* WorldContext);

// 공통 구현 매크로
#define DEFINE_SUBSYSTEM_GETTER_IMPL(ClassName) \
static ClassName* Get(UObject* WorldContext) \
{ \
	if (!WorldContext) \
		return nullptr; \
	if (auto* GI = WorldContext->GetWorld()->GetGameInstance()) \
		return GI->GetSubsystem<ClassName>(); \
	return nullptr; \
}

#define BIND_DYNAMIC_DELEGATE(DelegateType, Obj, ClassType, FuncName) \
[] (auto* InObj) { \
DelegateType Tmp; \
Tmp.BindUFunction(InObj, GET_FUNCTION_NAME_CHECKED(ClassType, FuncName)); \
return Tmp; \
} (Obj)