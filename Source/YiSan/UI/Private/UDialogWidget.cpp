// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDialogWidget.h"
#include "GameLogging.h"
#include "UDelayTaskManager.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UDialogWidget::NativeConstruct()
{
        Super::NativeConstruct();

        if (DialogBorder)
        {
                DialogBorder->SetVisibility(ESlateVisibility::Hidden);
        }
}

void UDialogWidget::ShowDialog(FString InString)
{
        PRINTLOG(TEXT("%s"), *InString);

        if (!DialogBorder || !DialogText)
        {
                return;
        }

        DialogBorder->SetVisibility(ESlateVisibility::Visible);
        DialogBorder->SetBrushColor(ActivateColor);

        DialogText->SetText(FText::FromString(InString));

        if (UDelayTaskManager* DelayManager = UDelayTaskManager::Get(GetWorld()))
        {
                DelayManager->Delay(this, 5, [&]()
                {
                        DialogText->SetText(FText::GetEmpty());
                        DialogBorder->SetBrushColor(DeactivateColor);
                });
        }
}
