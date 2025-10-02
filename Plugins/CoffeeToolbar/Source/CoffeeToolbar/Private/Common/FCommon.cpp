/**
 * @file FCommon.cpp
 * @brief Coffee Toolbar 플러그인을 위한 공용 헬퍼 유틸리티를 구현합니다.
 */
#include "Common/FCommon.h"
#include "Common/FToolbarStyle.h"
#include "Framework/Application/SlateApplication.h"

/** @brief 에디터 스타일 또는 플러그인 스타일 세트에서 Slate 아이콘을 찾아 반환합니다. */
const FSlateIcon FCommon::GetIcon(FName IconName)
{
    if (IconName.ToString().Contains(TEXT(".")))
    {
        return FSlateIcon(FAppStyle::GetAppStyleSetName(), IconName);
    }
    
    return FSlateIcon(FToolbarStyle::GetStyleSetName(), IconName);
}

/** @brief 현재 툴바 액션이 대상으로 삼는 월드를 반환합니다. */
UWorld* FCommon::GetActiveTargetWorld()
{
    if (GEditor && GEditor->PlayWorld)
        return GEditor->PlayWorld;
    return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
}