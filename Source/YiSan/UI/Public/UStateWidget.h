// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once
#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Blueprint/UserWidget.h"
#include "UStateWidget.generated.h"

/**
 * @class UStateWidget
 * @brief 상태(시간, 네트워크, 오디오)를 표시하는 UI 위젯
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

	UFUNCTION(BlueprintCallable, Category = "State|AudioCature")
	void OnNearBuilding(EBuildingType InBuildingType);
	
	UFUNCTION(BlueprintCallable, Category = "State|Quest")
	void OnUpdateQuest(EBuildingType InBuildingType);
	
protected:
	/** @brief 현재 시간을 표시하는 텍스트 */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
	class UTextBlock* CurrentTimeText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Quest")
	class UTextBlock* QuestTargetText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Location")
	class UTextBlock* NearTargetText = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Focus")
	class UTextBlock* FocusTargetText = nullptr;
	
	/** @brief 오디오 스펙트럼을 표시하는 프로그레스바. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Audio")
	class UProgressBar* SpectrumProgressBar = nullptr;

	/** @brief 로딩 스피너를 표시하는 이미지 */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Network")
	class UImage* LoadingSpinner = nullptr;

	/** @brief 로딩 스피너의 회전 속도 (초당 각도) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Network")
	float SpinnerRotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Focus")
	EBuildingType BuildingType = EBuildingType::None;
	
	/** @brief 시간 텍스트 업데이트 간격(0보다 큰 값으로 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "0.05", ClampMax = "5.0"))
	float TimeUpdateInterval = 0.25f;

	/** @brief 스펙트럼 감소 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.0"))
	float SpectrumDecaySpeed = 6.0f;

	/** @brief 스펙트럼 상승 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.0"))
	float SpectrumRiseSpeed = 12.0f;

	/** @brief 스펙트럼 스무딩 값 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float SpectrumSmoothing = 0.2f;

private:
	UPROPERTY()
	TObjectPtr<class UQuestManager> QuestManager;

	EBuildingType QuestDisplayType = EBuildingType::None;
	
	FTimerHandle UpdateTimerHandle;
	float SpectrumDisplayValue = 0.f;
};