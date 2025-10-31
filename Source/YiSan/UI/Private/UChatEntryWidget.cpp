// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatEntryWidget.h"

#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UChatEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TextBlock)
		TextBlock->SetText(FText::FromString(ChatMessageData.Message));

	SetupMessageAppearance(ChatMessageData.SpeakerType, ChatMessageData.PlayerIndex);

	this->ReverseHorizontalBox(ChatMessageData.SpeakerType == EChatMessageType::User);
}

void UChatEntryWidget::ReverseHorizontalBox(bool IsPlayer)
{
	if (!HorizontalBox || !BorderIcon || !ChatSizeBox)
	{
		return;
	}

	// 자식들 전부 제거
	HorizontalBox->ClearChildren();

	if (IsPlayer)
	{
		// 순서: 아이콘 왼쪽, 말풍선 오른쪽
		HorizontalBox->AddChildToHorizontalBox(BorderIcon);
		HorizontalBox->AddChildToHorizontalBox(ChatSizeBox);
	}
	else
	{
		// 순서: 말풍선 왼쪽, 아이콘 오른쪽
		HorizontalBox->AddChildToHorizontalBox(ChatSizeBox);
		HorizontalBox->AddChildToHorizontalBox(BorderIcon);
	}
}