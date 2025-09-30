#include "Common/FCommon.h"
#include "Common/FToolbarStyle.h"
#include "Framework/Application/SlateApplication.h"

const FSlateIcon FCommon::GetIcon(FName IconName)
{
    if (IconName.ToString().Contains(TEXT(".")))
    {
        return FSlateIcon(FAppStyle::GetAppStyleSetName(), IconName);
    }
    
    return FSlateIcon(FToolbarStyle::GetStyleSetName(), IconName);
}

UWorld* FCommon::GetActiveTargetWorld()
{
    if (GEditor && GEditor->PlayWorld)
        return GEditor->PlayWorld;
    return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
}