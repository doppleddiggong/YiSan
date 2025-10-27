// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UMainWidget.h"

#include "FComponentHelper.h"
#include "UBroadcastManager.h"

#include "USmallPopup.h"
#include "UMegaPopup.h"
#include "Components/AudioComponent.h"

#include "Components/EditableTextBox.h"
#include "Input/Reply.h"
#include "Kismet/GameplayStatics.h"
#include "EndingWidget.h" // EndingWidget 헤더 추가

#define ENDINGWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_EndingWidget.WBP_EndingWidget_C")

UMainWidget::UMainWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	EndingWidgetClass = FComponentHelper::LoadClass<UEndingWidget>(ENDINGWIDGET_PATH);
}

void UMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( APlayerController* PC = GetWorld()->GetFirstPlayerController() )
	{
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
	
	BroadcastManager = UBroadcastManager::Get(GetWorld());
	if (BroadcastManager)
	{
		BroadcastManager->OnNearBuilding.AddDynamic(this, &UMainWidget::OnNearBuilding);
		BroadcastManager->OnMegaPopupClosed.AddDynamic(this, &UMainWidget::OnMegaPopupClosed);
	}

	// Popup 초기 상태 설정 (공간 차지하지 않게 collapsed 로 성정)
	if (SmallPopupCtn)
		SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
	
	if (MegaPopupCtn)
		MegaPopupCtn->SetVisibility(ESlateVisibility::Collapsed);

	PlayBGM();

	// 엔딩 위젯 클래스가 설정
	if (EndingWidgetClass)
	{
		EndingWidgetInstance = CreateWidget<UEndingWidget>(this, EndingWidgetClass);
		if (EndingWidgetInstance)
		{
			EndingWidgetInstance->AddToViewport();
		}
	}
}
void UMainWidget::PlayBGM()
{
    if (!BGM_Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("BGM_Sound is null"));
        return;
    }

    BGM_AudioComp = UGameplayStatics::SpawnSound2D(
        this,
        BGM_Sound,
        1.0f,   // Volume
        1.0f,   // Pitch
        0.0f,   // StartTime
        nullptr // Concurrency
    );

    if (BGM_AudioComp)
    {
        BGM_AudioComp->Play();
    }
}


bool UMainWidget::IsMegaPopupVisible() const
{
	if ( MegaPopupCtn == nullptr )
		return false;
	return MegaPopupCtn->GetVisibility() == ESlateVisibility::Visible;
}

bool UMainWidget::IsSmallPopupVisible() const
{
	if ( SmallPopupCtn == nullptr )
		return false;
    return SmallPopupCtn->GetVisibility() == ESlateVisibility::Visible;
}

void UMainWidget::OnMegaPopupClosed()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
        return;

    if (IsMegaPopupVisible())
    {
        // megapopup 및 smallpopup 숨기자
        MegaPopupCtn->OnClose();
        MegaPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
        SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);

        // 입력 모드도 원래로 돌린다
        // if ( 입력모드...를...못하게 하는건가? )
            // APlayerControl::OnHold(true);
        BroadcastManager->SendPlayerControlState(true, nullptr);

        SmallPopupCtn->UpdateBuildingInfo(CurNearBuildingType);
    }
    else if (IsSmallPopupVisible())
    {
        // mega popup 표시
        MegaPopupCtn->SetVisibility(ESlateVisibility::Visible);
        SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);

        // 입력모드 전환 (player input 전화)
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(MegaPopupCtn->TakeWidget());

        BroadcastManager->SendPlayerControlState(false, MegaPopupCtn);

        MegaPopupCtn->UpdateBuildingInfo(CurNearBuildingType);
    }
}

void UMainWidget::OnNearBuilding(EBuildingType InBuildingType)
{
	if ( CurNearBuildingType == InBuildingType )
		return;

	if (IsMegaPopupVisible())
		return;

	CurNearBuildingType = InBuildingType;

    if (InBuildingType != EBuildingType::None)
    {
        SmallPopupCtn->SetVisibility(ESlateVisibility::Visible);
    	SmallPopupCtn->UpdateBuildingInfo(InBuildingType);
    }
    else
    {
        SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
    }
}