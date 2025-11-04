/**
 * @file UGameFunctionLibrary.cpp
 * @brief UGameFunctionLibrary의 동작을 구현합니다.
 */
#include "UGameFunctionLibrary.h"

TSubclassOf<UGameDamageType> UGameFunctionLibrary::GetDamageTypeClass(EDamageType InType)
{
    TSubclassOf<UGameDamageType> DamageTypeClass = UGameDamageType::StaticClass();
    if (DamageTypeClass)
    {
        UGameDamageType* DefaultDamageType = DamageTypeClass->GetDefaultObject<UGameDamageType>();
        if (DefaultDamageType)
            DefaultDamageType->DamageType = InType;
    }
    return DamageTypeClass;
}

