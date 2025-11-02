// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "USessionInfoWidget.h"
#include "UYisanOnlineSystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	btn_Join->OnClicked.AddDynamic(this, &USessionInfoWidget::OnClickJoin);
}

void USessionInfoWidget::SetSessionInfo(int32 index, FString sessionName)
{
	sessionIndex = index;
	textSessionName->SetText(FText::FromString(sessionName));
}

void USessionInfoWidget::OnClickJoin()
{
	UYisanOnlineSystem::Get(GetWorld())->JoinOtherSession(sessionIndex);
}
