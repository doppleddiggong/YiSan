// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDialogWidget.h"
#include "GameLogging.h"
#include "UBroadcastManger.h"
#include "UDelayTaskManager.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (auto EventManager = UBroadcastManger::Get(this))
		EventManager->OnToastMessage.AddDynamic(this, &UDialogWidget::ShowDialog);

	DialogBorder->SetVisibility(ESlateVisibility::Hidden);
}

void UDialogWidget::ShowDialog(FString InString)
{
	PRINTLOG(TEXT("%s"), *InString );

	DialogBorder->SetVisibility(ESlateVisibility::Visible);
	DialogBorder->SetBrushColor(ActivateColor);

	DialogText->SetText(FText::FromString( InString ));

	UDelayTaskManager::Get(GetWorld())->Delay(this, 5, [&]() 
	{
		DialogText->SetText(FText::GetEmpty());
		DialogBorder->SetBrushColor(DeactivateColor);
	});
}