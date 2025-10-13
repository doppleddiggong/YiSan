// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UStateWidget.generated.h"

class UHorizontalBox;
class UImage;
class UProgressBar;
class UTextBlock;

UENUM(BlueprintType)
enum class ENetworkInteractionState : uint8
{
        Idle            UMETA(DisplayName = "Idle"),
        Waiting         UMETA(DisplayName = "Waiting"),
        Requesting      UMETA(DisplayName = "Requesting"),
        Responding      UMETA(DisplayName = "Responding"),
        Completed       UMETA(DisplayName = "Completed"),
        Failed          UMETA(DisplayName = "Failed"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkInteractionStateChanged, ENetworkInteractionState, NewState);

UCLASS(BlueprintType, Blueprintable)
class YISAN_API UStateWidget : public UUserWidget
{
        GENERATED_BODY()

public:
        UStateWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

        virtual void NativePreConstruct() override;
        virtual void NativeConstruct() override;
        virtual void NativeDestruct() override;
        virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

        UFUNCTION(BlueprintCallable, Category = "State|Time")
        void SetUseSystemTime(bool bInUseSystemTime);

        UFUNCTION(BlueprintCallable, Category = "State|Network")
        void SetNetworkInteractionState(ENetworkInteractionState InState);

        UFUNCTION(BlueprintCallable, Category = "State|Audio")
        void StartAudioCapture();

        UFUNCTION(BlueprintCallable, Category = "State|Audio")
        void StopAudioCapture();

        UFUNCTION(BlueprintCallable, Category = "State|Audio")
        void RebuildSpectrumBars();

        UPROPERTY(BlueprintAssignable, Category = "State|Network")
        FOnNetworkInteractionStateChanged OnNetworkInteractionStateChanged;

protected:
        void HandleTimeUpdate();
        void RefreshTimeTexts();
        void UpdateDayNightWidgets(float DayNightProgress, bool bIsDaytime, float SecondsToTransition);
        void RefreshNetworkWidgets();
        void InitializeSpectrumBars();
        void UpdateSpectrumVisualization(float DeltaTime);

        float EvaluateGameTimeProgress(float& OutSecondsUntilTransition, bool& bOutIsDaytime) const;
        void BuildGameClockDisplay(FText& OutCurrentTimeText) const;
        FText BuildPhaseStatusText(float SecondsUntilTransition, bool bIsDaytime) const;

protected:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "60.0"))
        float GameDayLengthSeconds = 600.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "0.1", ClampMax = "0.9"))
        float DayRatio = 0.5f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time")
        bool bUseSystemTime = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "0.05", ClampMax = "5.0"))
        float TimeUpdateInterval = 0.25f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "4", ClampMax = "128"))
        int32 SpectrumBarCount = 32;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio")
        bool bAutoStartAudioCapture = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.0"))
        float SpectrumDecaySpeed = 6.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.0"))
        float SpectrumRiseSpeed = 12.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.001", ClampMax = "1.0"))
        float SpectrumSmoothing = 0.2f;

protected:
        UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
        UTextBlock* CurrentTimeText = nullptr;

        UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
        UTextBlock* DayNightStatusText = nullptr;

        UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
        UProgressBar* DayNightProgressBar = nullptr;

        UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Audio")
        UHorizontalBox* SpectrumContainer = nullptr;

        UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Network")
        UImage* LoadingSpinner = nullptr;

        UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Network")
        UTextBlock* NetworkStatusText = nullptr;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Network")
        ENetworkInteractionState NetworkState = ENetworkInteractionState::Idle;

private:
        void EnsureTimeTimer();

        void UpdateSpectrumFromAnalyzer();

private:
        FTimerHandle TimeUpdateTimerHandle;

        TArray<TWeakObjectPtr<UProgressBar>> SpectrumBars;
        TArray<float> SpectrumDisplayValues;

        TSharedPtr<class FStateAudioAnalyzer> AudioAnalyzer;
};

