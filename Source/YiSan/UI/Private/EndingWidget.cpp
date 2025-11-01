// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "EndingWidget.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "ABuilding.h"
#include "FComponentHelper.h"

void UEndingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// // endImage 위젯을 이름으로 찾아 바인딩하고, 처음에는 숨깁니다.
	// endImage = Cast<UImage>(GetWidgetFromName("endImage"));
	// if (endImage)
	// {
	// 	endImage->SetVisibility(ESlateVisibility::Hidden);
	// }
	//
	// // 월드에 있는 별주 건물을 찾습니다.
	// FindByeoljuBuilding();
	//
	// // 1초마다 EndingLocation 함수를 반복적으로 호출하는 타이머를 설정합니다.
	// GetWorld()->GetTimerManager().SetTimer(EndingTimerHandle, this, &UEndingWidget::EndingLocation, 1.0f, true);
}
//
// void UEndingWidget::FindByeoljuBuilding()
// {
// 	// 월드에 있는 모든 ABuilding 액터를 찾습니다.
// 	auto FoundBuildings = FComponentHelper::GetAllOfClass<ABuilding>(GetWorld());
// 	for (ABuilding* Building : FoundBuildings)
// 	{
// 		// 건물의 타입이 별주 인지 확인합니다.
// 		if (Building && Building->BuildingType == EBuildingType::Byeolju)
// 		{
// 			// 별주 건물을 찾았으면, 멤버 변수에 저장하고 루프를 종료합니다.
// 			ByeoljuBuilding = Building;
// 			break;
// 		}
// 	}
// }
//
// void UEndingWidget::EndingLocation()
// {
// 	// 별주 건물이 월드에 존재하는지, 그리고 endImage 위젯이 유효한지 확인합니다.
// 	if (!ByeoljuBuilding || !endImage)
// 	{
// 		return;
// 	}
//
// 	// 이미지가 이미 보이면, 더 이상 처리할 필요가 없습니다.
// 	if (endImage->GetVisibility() == ESlateVisibility::Visible)
// 	{
// 		// 타이머를 정지시켜 불필요한 반복을 막습니다.
// 		GetWorld()->GetTimerManager().ClearTimer(EndingTimerHandle);
// 		return;
// 	}
//
// 	// 플레이어 캐릭터를 가져옵니다.
// 	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
// 	if (PlayerCharacter)
// 	{
// 		// 플레이어와 별주 건물 사이의 거리를 계산합니다.
// 		float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), ByeoljuBuilding->GetActorLocation());
//
// 		// 거리가 설정된 값(CheckDistance)보다 가까우면
// 		if (Distance <= CheckDistance)
// 		{
// 			// 이미지를 보이게 합니다.
// 			endImage->SetVisibility(ESlateVisibility::Visible);
// 		}
// 	}
// }