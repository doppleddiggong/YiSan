// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once
#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Blueprint/UserWidget.h"
#include "UStateWidget.generated.h"

/// @file UStateWidget.h
/// @brief 시간·네트워크·오디오 상태를 표시하는 UI 위젯을 선언합니다.
/// @brief 시간, 네트워크 대기, 오디오 스펙트럼을 시각화하는 상태 패널입니다.

UCLASS(BlueprintType, Blueprintable)
class YISAN_API UStateWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /// @brief 위젯 생성 시 초기화 작업을 수행합니다.
    virtual void NativeConstruct() override;

    /// @brief 위젯 파괴 시 타이머와 델리게이트를 정리합니다.
    virtual void NativeDestruct() override;

    /// @brief 매 프레임 상태 표시를 갱신합니다.
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    /// @brief 현재 시간을 포맷팅해 텍스트로 출력합니다.
    UFUNCTION()
    void RefreshTimeText();

    /// @brief 오디오 스펙트럼 표시 값을 업데이트합니다.
    void UpdateSpectrumVisual(float DeltaTime);

    /// @brief 로딩 스피너 회전 애니메이션을 갱신합니다.
    void UpdateLoadingSpinner(float DeltaTime);

    /// @brief 네트워크 대기 카운트 변경 시 호출됩니다.
    /// @param NetworkWaitCount [in] 처리 중인 요청 수입니다.
    UFUNCTION(BlueprintCallable, Category = "State|Network")
    void OnNetworkWaitCount(int NetworkWaitCount);

    /// @brief 녹음 시작/종료 상태를 표시합니다.
    UFUNCTION(BlueprintCallable, Category = "State|AudioCature")
    void OnAudioCapture(bool bRecording);

    /// @brief 실시간 오디오 스펙트럼 값을 반영합니다.
    UFUNCTION(BlueprintCallable, Category = "State|AudioCature")
    void OnAudioSpectrum(float Spectrum);

    /// @brief 플레이어 시선에 들어온 건물 정보를 표시합니다.
    UFUNCTION(BlueprintCallable, Category = "State|AudioCature")
    void OnFocusBuilding(EBuildingType InBuildingType);

    /// @brief 주변 건물 정보를 표시합니다.
    UFUNCTION(BlueprintCallable, Category = "State|AudioCature")
    void OnNearBuilding(EBuildingType InBuildingType);

    /// @brief 퀘스트 목표 변화를 표시합니다.
    UFUNCTION(BlueprintCallable, Category = "State|Quest")
    void OnUpdateQuest(EBuildingType InBuildingType);

protected:
    /// @brief 현재 시간을 표시하는 텍스트 위젯입니다.
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
    class UTextBlock* CurrentTimeText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Quest")
    class UTextBlock* QuestTargetText = nullptr;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Quest")
    class UImage* QuestTargetImage = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Location")
    class UTextBlock* NearTargetText = nullptr;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Location")
    class UImage* NearTargetImage = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Focus")
    class UTextBlock* FocusTargetText = nullptr;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Focus")
    class UImage* FocusTargetImage = nullptr;

    /// @brief 오디오 스펙트럼을 시각화하는 프로그레스바입니다.
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Audio")
    class UProgressBar* SpectrumProgressBar = nullptr;

    /// @brief 네트워크 대기 상태를 나타내는 로딩 스피너 이미지입니다.
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Network")
    class UImage* LoadingSpinner = nullptr;

    /// @brief 로딩 스피너의 회전 속도(초당 각도)입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Network")
    float SpinnerRotationSpeed = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Focus")
    EBuildingType BuildingType = EBuildingType::None;

    /// @brief 시간 표시를 갱신하는 간격(초)입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "0.05", ClampMax = "5.0"))
    float TimeUpdateInterval = 0.25f;

    /// @brief 스펙트럼 값이 감소할 때의 속도입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.0"))
    float SpectrumDecaySpeed = 6.0f;

    /// @brief 스펙트럼 값이 상승할 때의 속도입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.0"))
    float SpectrumRiseSpeed = 12.0f;

    /// @brief 스펙트럼 값을 스무딩하기 위한 보간 비율입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio", meta = (ClampMin = "0.001", ClampMax = "1.0"))
    float SpectrumSmoothing = 0.2f;

    EBuildingType QuestDisplayType = EBuildingType::None;

    FTimerHandle UpdateTimerHandle;
    float SpectrumDisplayValue = 0.f;
};
