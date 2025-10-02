/**
 * @file UGameFunctionLibrary.cpp
 * @brief UGameFunctionLibrary 구현에 대한 Doxygen 주석을 제공합니다.
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

