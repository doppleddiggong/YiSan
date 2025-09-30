#include "Command/FCommandFeature.h"
#include "Settings/UToolbarSettings.h"
#include "Common/FCommon.h"
#include "ToolMenus.h"

FCommandFeature::FCommandFeature()
{
}

TSharedRef<SWidget> FCommandFeature::GenerateCommandsMenu()
{
    FMenuBuilder MenuBuilder(true, nullptr);

    const UToolbarSettings* Settings = GetDefault<UToolbarSettings>();
    for (const auto& ButtonInfo : Settings->ToolbarButtons)
    {
        FUIAction Action(
            FExecuteAction::CreateRaw(this, &FCommandFeature::OnExecuteButtonCommand, ButtonInfo.Id),
            FCanExecuteAction(),
            FIsActionChecked::CreateRaw(this, &FCommandFeature::IsButtonToggled, ButtonInfo.Id)
        );

        MenuBuilder.AddMenuEntry(
            FText::FromString(ButtonInfo.Label),
            FText::FromString(ButtonInfo.Tooltip),
            FCommon::GetIcon(ButtonInfo.Icon),
            Action,
            NAME_None,
            ButtonInfo.bIsToggleButton ? EUserInterfaceActionType::ToggleButton : EUserInterfaceActionType::Button
        );
    }

    return MenuBuilder.MakeWidget();
}

void FCommandFeature::OnExecuteButtonCommand(FName ButtonId)
{
    const UToolbarSettings* Settings = GetDefault<UToolbarSettings>();
    const FToolbarButtonInfo* ButtonInfo = Settings->ToolbarButtons.FindByPredicate([&](const FToolbarButtonInfo& Button){ return Button.Id == ButtonId; });

    if (!ButtonInfo)
        return;

    if (UWorld* W = FCommon::GetActiveTargetWorld())
    {
        FString Command = ButtonInfo->Command;
        if (ButtonInfo->bIsToggleButton)
        {
            bool& bIsToggled = ToggleButtonState.FindOrAdd(ButtonId);
            bIsToggled = !bIsToggled;

            if (Command.Contains(TEXT(" ")))
            {
                Command.Append(bIsToggled ? TEXT(" 1") : TEXT(" 0"));
            }
        }
        UE_LOG(LogTemp, Log, TEXT("Executing command: %s"), *Command);
        GEditor->Exec(W, *Command);
    }
}

bool FCommandFeature::IsButtonToggled(FName ButtonId) const
{
    return ToggleButtonState.FindRef(ButtonId);
}
