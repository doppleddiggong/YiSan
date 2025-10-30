// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UPlayerHeadWidget.h"

#include "APlayerActor.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "AYiSanPlayerState.h"
#include "UChatPlayerSystem.h"
#include "GameFramework/Pawn.h"

void UPlayerHeadWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPlayerHeadWidget::SetOwningActor(APlayerActor* Actor)
{
	OwningActor = Actor;

	// 주인이 설정되면, 닉네임 확인을 위한 타이머를 시작합니다.
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_CheckPlayerState, this, &UPlayerHeadWidget::CheckPlayerState, 0.2f, true);
}

void UPlayerHeadWidget::CheckPlayerState()
{
	// 유효한 텍스트 블록과 소유 액터가 있을 경우에만 로직 실행
	if (OwningActor.IsValid())
	{
		// Pawn에서 PlayerState를 가져와 AYiSanPlayerState로 캐스팅합니다.
		if (auto PS = OwningActor->GetPlayerState<AYiSanPlayerState>())
		{
			// 닉네임이 비어있지 않다면 텍스트를 설정합니다.
			if (!PS->Nickname.IsEmpty())
			{
				// 이름이 설정되었으므로, 타이머를 정지시킵니다.
				GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CheckPlayerState);
				
				PlayerNameText->SetText(FText::FromString(PS->Nickname));
				image_Icon->SetBrushFromTexture( LoadObject<UTexture2D>(nullptr, *PS->GetResourcePath()));
				OwningActor->ChatPlayerSystem->AnnouncePlayerJoin();
			}
		}
	}
}