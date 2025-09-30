#pragma once

#include "CoreMinimal.h"
#include "Styling/AppStyle.h"

class FCommon
{
public:
    static const FSlateIcon GetIcon(FName IconName);
    static UWorld* GetActiveTargetWorld();
};