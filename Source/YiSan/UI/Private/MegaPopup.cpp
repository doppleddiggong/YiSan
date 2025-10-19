// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "MegaPopup.h"
#include "Components/TextBlock.h"

void UMegaPopup::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMegaPopup::SetDescription(const FString& InText)
{
	if (Description)
		Description->SetText(FText::FromString(InText));
}

void UMegaPopup::ShowLoading(bool bShow)
{	
	if (Description && bShow)
		Description->SetText(FText::FromString(TEXT("GPT가 설명을 생성하고 있습니다...")));
}