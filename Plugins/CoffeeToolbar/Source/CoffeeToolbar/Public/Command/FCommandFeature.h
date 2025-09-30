#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Framework/Commands/UIAction.h"

class FCommandFeature
{
public:
    FCommandFeature();

    TSharedRef<SWidget> GenerateCommandsMenu();
    void OnExecuteButtonCommand(FName ButtonId);
    bool IsButtonToggled(FName ButtonId) const;

private:
    TMap<FName, bool> ToggleButtonState;
};