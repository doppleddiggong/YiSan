// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStateWidget.h"

#include "ABuilding.h"
#include "AQuestManagerActor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/CriticalSection.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeLock.h"
#include "TimerManager.h"
#include "UBroadcastManager.h"
#include "UBuildingDetailData.h"
#include "UGameDataManager.h"

void UStateWidget::NativeConstruct()
{
    Super::NativeConstruct();

    QuestDisplayType = EBuildingType::None;

    LoadingSpinner->SetVisibility(ESlateVisibility::Hidden);
    SpectrumProgressBar->SetVisibility(ESlateVisibility::Hidden);
    
    QuestTargetText->SetText(FText::GetEmpty());
    QuestTargetText->SetVisibility(ESlateVisibility::Hidden);
    QuestTargetImage->SetVisibility(ESlateVisibility::Hidden);
    QuestArrowImage->SetVisibility(ESlateVisibility::Hidden);

    NearTargetText->SetText(FText::GetEmpty());
    NearTargetText->SetVisibility(ESlateVisibility::Hidden);
    NearTargetImage->SetVisibility(ESlateVisibility::Hidden);
    
    if (UWorld* World = GetWorld())
        World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UStateWidget::RefreshTimeText, TimeUpdateInterval, true);

    if (AQuestManagerActor* QuestActor = AQuestManagerActor::Get(this))
    {
        QuestManager = QuestActor;
        OnUpdateQuest(QuestActor->GetCurrentTarget());
    }
    
    if ( auto EventManager = UBroadcastManager::Get(GetWorld()))
    {
        EventManager->OnNetworkWaitCount.AddDynamic(this, &UStateWidget::OnNetworkWaitCount);
        EventManager->OnAudioCapture.AddDynamic(this, &UStateWidget::OnAudioCapture);
        EventManager->OnAudioSpectrum.AddDynamic(this, &UStateWidget::OnAudioSpectrum);
        EventManager->OnUpdateQuest.AddDynamic(this, &UStateWidget::OnUpdateQuest);
        EventManager->OnNearBuilding.AddDynamic(this, &UStateWidget::OnNearBuilding);
    }
}

void UStateWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);

    Super::NativeDestruct();
}

void UStateWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateSpectrumVisual(InDeltaTime);
    UpdateLoadingSpinner(InDeltaTime);
    UpdateQuestArrow();
}

void UStateWidget::RefreshTimeText()                                                                                                                                                                     
{
    const FDateTime Now = FDateTime::Now();
    const FString TimeString = Now.ToString(TEXT("%H:%M:%S"));
    const FText TimeText = FText::FromString(TimeString);                                                                                                                           

    CurrentTimeText->SetText(TimeText);                                                                                                                                       
}          

void UStateWidget::UpdateSpectrumVisual(float DeltaTime)
{
    if ( !SpectrumProgressBar->IsVisible())
        return;
    
    const float Delta = FMath::Max(DeltaTime, 0.0f);
    const float TargetValue = SpectrumDisplayValue;
    const float CurrentValue = SpectrumProgressBar->GetPercent();
    const float InterpSpeed = TargetValue > CurrentValue ? SpectrumRiseSpeed : SpectrumDecaySpeed;
    const float NewPercent = FMath::FInterpConstantTo(CurrentValue, TargetValue, Delta, InterpSpeed);
    SpectrumProgressBar->SetPercent(FMath::Clamp(NewPercent, 0.0f, 1.0f));
}

void UStateWidget::UpdateLoadingSpinner(float DeltaTime)
{
    if ( !LoadingSpinner->IsVisible())
        return;
    
    const float NewAngle = LoadingSpinner->GetRenderTransformAngle() + (SpinnerRotationSpeed * DeltaTime);
    LoadingSpinner->SetRenderTransformAngle(NewAngle);
}

void UStateWidget::OnNetworkWaitCount(int NetworkWaitCount)
{
    LoadingSpinner->SetVisibility( NetworkWaitCount > 0 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UStateWidget::OnAudioCapture(bool bRecording)
{
    SpectrumProgressBar->SetVisibility( bRecording ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UStateWidget::OnAudioSpectrum(float Spectrum)
{
    SpectrumDisplayValue = Spectrum;
}

void UStateWidget::OnUpdateQuest(EBuildingType InBuildingType)
{
    if (!QuestTargetText || !QuestTargetImage || !QuestArrowImage)
        return;

    if (InBuildingType != EBuildingType::None)
        LastQuestTarget = InBuildingType;

    const bool bDisplayLastByeolju = (InBuildingType == EBuildingType::None && LastQuestTarget == EBuildingType::Byeolju);
    const EBuildingType DisplayType = bDisplayLastByeolju ? LastQuestTarget : InBuildingType;

    QuestDisplayType = DisplayType;

    if (DisplayType == EBuildingType::None)
    {
        QuestTargetText->SetText(FText::GetEmpty());
        QuestTargetText->SetVisibility(ESlateVisibility::Hidden);
        QuestTargetImage->SetVisibility(ESlateVisibility::Hidden);
        QuestArrowImage->SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    auto GameDataManager = UGameDataManager::Get(GetWorld());
    auto BuildingName  = GameDataManager->GetBuildingDataName(DisplayType);
    QuestTargetText->SetText(FText::FromString(BuildingName));
    QuestTargetText->SetVisibility(ESlateVisibility::Visible);
    QuestArrowImage->SetVisibility(ESlateVisibility::Visible);
    QuestArrowImage->SetVisibility(bDisplayLastByeolju ? ESlateVisibility::Hidden : ESlateVisibility::Visible);

    FBuildingAssetData AssetData;
    if ( GameDataManager->GetBuildingAssetData(DisplayType, AssetData) )
    {
        UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();
        if ( DetailAsset )
        {
            TSoftObjectPtr<UTexture2D> LoadedTexture;
            if ( DetailAsset->LoadThumbnailImage(LoadedTexture) && LoadedTexture.Get() )
            {
                QuestTargetImage->SetBrushFromTexture(LoadedTexture.Get());
                QuestTargetImage->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}

void UStateWidget::UpdateQuestArrow()
{
    // 퀘스트가 없으면 화살표 숨김
    if (QuestDisplayType == EBuildingType::None || !QuestArrowImage)
        return;

    // 플레이어 컨트롤러 가져오기
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
        return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
        return;

    // 목표 건물 찾기
    ABuilding* TargetBuilding = nullptr;
    TArray<AActor*> FoundBuildings;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), FoundBuildings);

    for (AActor* Actor : FoundBuildings)
    {
        ABuilding* Building = Cast<ABuilding>(Actor);
        if (Building && Building->BuildingType == QuestDisplayType)
        {
            TargetBuilding = Building;
            break;
        }
    }

    if (!TargetBuilding)
        return;

    // 플레이어 위치와 목표 건물 위치
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector TargetLocation = TargetBuilding->GetActorLocation();

    // 플레이어에서 목표로의 방향 벡터 (2D 평면)
    FVector DirectionToTarget = TargetLocation - PlayerLocation;
    DirectionToTarget.Z = 0; // 수평 방향만 고려
    DirectionToTarget.Normalize();

    // 플레이어의 정면 방향 (카메라 기준)
    FVector CameraForward = PC->GetControlRotation().Vector();
    CameraForward.Z = 0;
    CameraForward.Normalize();

    // 두 벡터 사이의 각도 계산 (Degree)
    float DotProduct = FVector::DotProduct(CameraForward, DirectionToTarget);
    float CrossProduct = FVector::CrossProduct(CameraForward, DirectionToTarget).Z;

    // 각도 계산 (-180 ~ 180)
    float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

    // CrossProduct의 부호로 방향 결정
    if (CrossProduct < 0)
        AngleDegrees = -AngleDegrees;

    // 화살표 회전 (화살표 머리가 12시 방향이므로 그대로 적용)
    QuestArrowImage->SetRenderTransformAngle(AngleDegrees);
}

void UStateWidget::OnNearBuilding(EBuildingType InBuildingType)
{
    if (InBuildingType == EBuildingType::None)
    {
        NearTargetText->SetText(FText::GetEmpty());
        NearTargetText->SetVisibility(ESlateVisibility::Hidden);
        NearTargetImage->SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    
    auto GameDataManager = UGameDataManager::Get(GetWorld());
    auto BuildingName  = GameDataManager->GetBuildingDataName(InBuildingType);
    NearTargetText->SetText(FText::FromString(BuildingName));
    NearTargetText->SetVisibility(ESlateVisibility::Visible);

    FBuildingAssetData AssetData;
    if ( GameDataManager->GetBuildingAssetData(InBuildingType, AssetData) )
    {
        UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();
        if ( DetailAsset )
        {
            TSoftObjectPtr<UTexture2D> LoadedTexture;
            if ( DetailAsset->LoadThumbnailImage(LoadedTexture) && LoadedTexture.Get() )
            {
                NearTargetImage->SetBrushFromTexture(LoadedTexture.Get());
                NearTargetImage->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}