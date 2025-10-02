/**
 * @file FCommandFeature.cpp
 * @brief Coffee Toolbar용 명령 실행 보조 기능을 구현합니다.
 */
#include "Command/FCommandFeature.h"
#include "Settings/UToolbarSettings.h"
#include "Common/FCommon.h"
#include "ToolMenus.h"

/** @brief 메뉴 상태를 생성하지 않고 기능 객체를 초기화합니다. */
FCommandFeature::FCommandFeature()
{
}

/** @brief 설정된 툴바 버튼 정보를 바탕으로 메뉴 위젯을 구성합니다. */
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

/** @brief 전달된 툴바 버튼에 연결된 명령을 실행합니다. */
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

/** @brief 지정된 토글 버튼이 현재 활성화되어 있는지 반환합니다. */
bool FCommandFeature::IsButtonToggled(FName ButtonId) const
{
    return ToggleButtonState.FindRef(ButtonId);
}
