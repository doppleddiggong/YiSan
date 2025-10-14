// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once
#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Blueprint/UserWidget.h"
#include "UStateWidget.generated.h"

/**
 * @class UStateWidget
 * @brief 다양한 상태(시간, 네트워크, 오디오 스펙트럼)를 시각적으로 표시하는 UI 위젯입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class YISAN_API UStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void RefreshTimeText();

	void UpdateSpectrumVisual(float DeltaTime);
	void UpdateLoadingSpinner(float DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category = "State|Network")
	void OnNetworkWaitCount(int NetworkWaitCount);

	UFUNCTION(BlueprintCallable, Category = "State|AudioCature")
	void OnAudioCapture(bool bRecording);

	UFUNCTION(BlueprintCallable, Category = "State|AudioCature")
	void OnAudioSpectrum(float Spectrum);

	UFUNCTION(BlueprintCallable, Category = "State|AudioCature")
	void OnFocusBuilding(EBuildingType InBuildingType);
	
protected:
	/** @brief 현재 시간을 표시하는 텍스트 블록입니다. */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
	class UTextBlock* CurrentTimeText = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Focus")
	class UTextBlock* FocusBuildingText = nullptr;

	/** @brief 오디오 스펙트럼을 표시하는 진행률 표시줄입니다. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Audio")
	class UProgressBar* SpectrumProgressBar = nullptr;

	/** @brief 로딩 중임을 나타내는 스피너 이미지입니다. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Network")
	class UImage* LoadingSpinner = nullptr;

	/** @brief 로딩 스피너의 회전 속도입니다. (초당 각도) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Network")
	float SpinnerRotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Focus")
	EBuildingType BuildingType = EBuildingType::None;
	
	/** @brief 시간 텍스트를 업데이트하는 간격(초)입니다. 0 또는 음수이면 매 틱마다 업데이트됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "0.05", ClampMax = "5.0"))
	float TimeUpdateInterval = 0.25f;

	/** @brief 스펙트럼 시각화가 감소하는 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.0"))
	float SpectrumDecaySpeed = 6.0f;

	/** @brief 스펙트럼 시각화가 상승하는 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.0"))
	float SpectrumRiseSpeed = 12.0f;

	/** @brief 스펙트럼 값의 부드러움을 조절합니다. (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float SpectrumSmoothing = 0.2f;

private:
	FTimerHandle UpdateTimerHandle;
	float SpectrumDisplayValue = 0.f;
};