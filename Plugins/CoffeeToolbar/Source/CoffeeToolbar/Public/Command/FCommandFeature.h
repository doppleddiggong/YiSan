#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Framework/Commands/UIAction.h"

/**
 * @file FCommandFeature.h
 * @brief 명령형 버튼을 담당하는 툴바 기능을 선언합니다.
 */

/**
 * @brief 툴바 명령 버튼을 위해 Slate 메뉴를 구성하고 실행 상태를 관리합니다.
 */
class FCommandFeature
{
public:
    /** @brief 기본 토글 상태를 초기화하고 명령 매핑을 준비합니다. */
    FCommandFeature();

    /** @brief 툴바용 명령 버튼 메뉴 위젯을 생성합니다. */
    TSharedRef<SWidget> GenerateCommandsMenu();

    /** @brief 전달된 버튼 ID에 매핑된 명령을 실행합니다. */
    void OnExecuteButtonCommand(FName ButtonId);

    /** @brief 지정된 툴바 버튼이 현재 토글되어 있는지 반환합니다. */
    bool IsButtonToggled(FName ButtonId) const;

private:
    /** @brief 버튼 식별자를 키로 사용하는 명령 버튼의 토글 상태 캐시입니다. */
    TMap<FName, bool> ToggleButtonState;
};