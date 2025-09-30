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

