// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UPlayerHeadWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "AYiSanPlayerState.h"
#include "GameFramework/Pawn.h"

void UPlayerHeadWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPlayerHeadWidget::SetOwningActor(AActor* Actor)
{
	OwningActor = Actor;

	// 주인이 설정되면, 닉네임 확인을 위한 타이머를 시작합니다.
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_CheckPlayerState, this, &UPlayerHeadWidget::CheckPlayerState, 0.2f, true);
}

void UPlayerHeadWidget::CheckPlayerState()
{
	// 유효한 텍스트 블록과 소유 액터가 있을 경우에만 로직 실행
	if (PlayerNameText && OwningActor.IsValid())
	{
		APawn* OwningPawn = Cast<APawn>(OwningActor.Get());

		if (OwningPawn)
		{
			// Pawn에서 PlayerState를 가져와 AYiSanPlayerState로 캐스팅합니다.
			AYiSanPlayerState* PS = OwningPawn->GetPlayerState<AYiSanPlayerState>();
			if (PS)
			{
				// PlayerState에서 Nickname을 가져옵니다.
				const FString Nickname = PS->Nickname;

				// 닉네임이 비어있지 않다면 텍스트를 설정합니다.
				if (!Nickname.IsEmpty())
				{
					PlayerNameText->SetText(FText::FromString(Nickname));

					// index도 가져오자
					int32 index = PS->PlayerIndex;
					if (index < 0) return;
					switch (index)
					{
					case 0:
						image_Icon->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_red")));
						break;
					case 1:
						image_Icon->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_green")));
						break;
					case 2:
						image_Icon->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_blue")));
						break;
					case 3:
						image_Icon->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_black")));
						break;
					case 4:
						image_Icon->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_white")));
						break;
					}
					// 이름이 설정되었으므로, 타이머를 정지시킵니다.
					GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CheckPlayerState);
				}
			}
		}
	}
}

void UPlayerHeadWidget::SetPlayerName(const FString& InPlayerName)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(InPlayerName));
	}
}