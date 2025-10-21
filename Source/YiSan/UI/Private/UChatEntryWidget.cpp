// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatEntryWidget.h"
#include "Components/TextBlock.h"

void UChatEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TextBlock)
		TextBlock->SetText(FText::FromString(Message));
}
