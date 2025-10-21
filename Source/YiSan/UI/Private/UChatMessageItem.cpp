// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UChatMessageItem.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

void UChatMessageItem::SetMessageData(const FString& Sender, const FString& Message, const FLinearColor& Color)
{
	// 발화자 이름 설정
	if (SenderText)
	{
		FString SenderDisplay = FString::Printf(TEXT("[%s]"), *Sender);
		SenderText->SetText(FText::FromString(SenderDisplay));
		SenderText->SetColorAndOpacity(FSlateColor(Color));
	}

	// 메시지 내용 설정
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(Message));
		MessageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}

	// 타임스탬프 설정 (옵션)
	if (TimestampText)
	{
		FDateTime Now = FDateTime::Now();
		FString TimeString = FString::Printf(TEXT("%02d:%02d"), Now.GetHour(), Now.GetMinute());
		TimestampText->SetText(FText::FromString(TimeString));
		TimestampText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)));
	}

	// 아이콘 설정 (옵션)
	if (MessageIcon)
	{
		// TODO: Sender에 따라 다른 아이콘 설정
		// 예: Dasan이면 Dasan 아이콘, Player면 플레이어 아이콘
		if (Sender == TEXT("Dasan"))
		{
			// MessageIcon->SetBrushFromTexture(DasanIconTexture);
			MessageIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			MessageIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}