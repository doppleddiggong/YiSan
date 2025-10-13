// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once
#include "CoreMinimal.h"
#include "ENetworkState.h"
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
	/**
	 * @brief 위젯이 에디터에서 생성되거나 디자인 타임에 속성이 변경될 때 호출됩니다.
	 */
	virtual void NativePreConstruct() override;

	/**
	 * @brief 위젯이 런타임에 처음으로 생성될 때 호출됩니다.
	 */
	virtual void NativeConstruct() override;

	/**
	 * @brief 위젯이 소멸될 때 호출됩니다.
	 */
	virtual void NativeDestruct() override;

	/**
	 * @brief 매 틱마다 호출됩니다.
	 * @param MyGeometry 위젯의 지오메트리 정보입니다.
	 * @param InDeltaTime 마지막 틱 이후 경과된 시간입니다.
	 */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/**
	 * @brief 네트워크 상태에 해당하는 문자열을 반환합니다.
	 * @param InState 네트워크 상태입니다.
	 * @return 네트워크 상태 문자열입니다.
	 */
	FORCEINLINE FString GetNetworkStateString(ENetworkState InState)
	{
		switch (InState)
		{
		case ENetworkState::Idle:			return TEXT("대기중");
		case ENetworkState::Waiting:		return TEXT("대기중");
		case ENetworkState::Requesting:		return TEXT("요청중");
		case ENetworkState::Responding:		return TEXT("응답중");
		case ENetworkState::Completed:		return TEXT("완료");
		case ENetworkState::Failed:			return TEXT("실패");
		default:
			return TEXT("알수없음");
		}
	}

	/**
	 * @brief 로딩 스피너가 보여야 하는지 여부를 결정합니다.
	 * @param InState 네트워크 상태입니다.
	 * @return 로딩 스피너가 보여야 하면 true, 그렇지 않으면 false를 반환합니다.
	 */
	FORCEINLINE bool ShouldSpinnerBeVisible(ENetworkState InState)
	{
		return InState == ENetworkState::Waiting || InState == ENetworkState::Requesting || InState == ENetworkState::Responding;
	}
	
	/**
	 * @brief 시스템 시간을 사용할지 여부를 설정합니다.
	 * @param bInUseSystemTime 시스템 시간을 사용하려면 true로 설정합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Time")
	void SetUseSystemTime(bool bInUseSystemTime);

	/**
	 * @brief 오디오 캡처를 시작합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Audio")
	void StartAudioCapture();

	/**
	 * @brief 오디오 캡처를 중지합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Audio")
	void StopAudioCapture();

protected:
	/**
	 * @brief 시간 업데이트를 처리합니다.
	 */
	void HandleTimeUpdate();

	/**
	 * @brief 시간 텍스트를 새로 고칩니다.
	 */
	void RefreshTimeTexts();

	/**
	 * @brief 낮/밤 위젯을 업데이트합니다.
	 * @param DayNightProgress 낮/밤 주기의 진행률입니다. (0.0 ~ 1.0)
	 * @param bIsDaytime 현재 낮인지 여부입니다.
	 * @param SecondsToTransition 다음 상태 전환까지 남은 시간(초)입니다.
	 */
	void UpdateDayNightWidgets(float DayNightProgress, bool bIsDaytime, float SecondsToTransition);

	/**
	 * @brief 네트워크 관련 위젯을 새로 고칩니다.
	 */
	void RefreshNetworkWidgets();

	/**
	 * @brief 오디오 스펙트럼 시각화를 업데이트합니다.
	 * @param DeltaTime 마지막 프레임 이후 경과된 시간입니다.
	 */
	void UpdateSpectrumVisualization(float DeltaTime);

	/**
	 * @brief 게임 내 시간 진행률을 평가합니다.
	 * @param OutSecondsUntilTransition 다음 상태 전환까지 남은 시간(초)을 반환합니다.
	 * @param bOutIsDaytime 현재 낮인지 여부를 반환합니다.
	 * @return 현재 주기의 진행률을 반환합니다. (0.0 ~ 1.0)
	 */
	float EvaluateGameTimeProgress(float& OutSecondsUntilTransition, bool& bOutIsDaytime) const;

	/**
	 * @brief 게임 시계 표시 텍스트를 빌드합니다.
	 * @param OutCurrentTimeText 현재 시간 텍스트를 반환합니다.
	 */
	void BuildGameClockDisplay(FText& OutCurrentTimeText) const;

	/**
	 * @brief 단계 상태 텍스트를 빌드합니다. (예: "낮 | 전환까지 01:23")
	 * @param SecondsUntilTransition 다음 상태 전환까지 남은 시간(초)입니다.
	 * @param bIsDaytime 현재 낮인지 여부입니다.
	 * @return 빌드된 텍스트를 반환합니다.
	 */
	FText BuildPhaseStatusText(float SecondsUntilTransition, bool bIsDaytime) const;

private:
	/**
	 * @brief 시간 업데이트 타이머가 설정되었는지 확인하고, 필요하면 설정합니다.
	 */
	void EnsureTimeTimer();

	/**
	 * @brief 오디오 분석기에서 스펙트럼 데이터를 업데이트합니다.
	 */
	void UpdateSpectrumFromAnalyzer();

	/**
	 * @brief 네트워크 상태가 변경될 때 호출됩니다.
	 * @param InState 새로운 네트워크 상태입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Network")
	void OnNetworkStateChanged(ENetworkState InState);


protected:
	/** @brief 현재 시간을 표시하는 텍스트 블록입니다. */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
	class UTextBlock* CurrentTimeText = nullptr;

	/** @brief 낮/밤 상태를 표시하는 텍스트 블록입니다. */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
	class UTextBlock* DayNightStatusText = nullptr;

	/** @brief 낮/밤 주기를 표시하는 진행률 표시줄입니다. */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "State|Time")
	class UProgressBar* DayNightProgressBar = nullptr;

	/** @brief 오디오 스펙트럼을 표시하는 진행률 표시줄입니다. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Audio")
	class UProgressBar* SpectrumProgressBar = nullptr;

	/** @brief 로딩 중임을 나타내는 스피너 이미지입니다. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Network")
	class UImage* LoadingSpinner = nullptr;

	/** @brief 네트워크 상태를 표시하는 텍스트 블록입니다. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Network")
	class UTextBlock* NetworkStatusText = nullptr;

	/** @brief 현재 네트워크 상호작용 상태입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Network")
	ENetworkState NetworkState = ENetworkState::Idle;

	/** @brief 게임 내 하루의 길이(초)입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "60.0"))
	float GameDayLengthSeconds = 600.0f;

	/** @brief 전체 하루 중 낮이 차지하는 비율입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "0.1", ClampMax = "0.9"))
	float DayRatio = 0.5f;

	/** @brief 시스템 시간을 사용할지 여부입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time")
	bool bUseSystemTime = false;

	/** @brief 시간 텍스트를 업데이트하는 간격(초)입니다. 0 또는 음수이면 매 틱마다 업데이트됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Time", meta = (ClampMin = "0.05", ClampMax = "5.0"))
	float TimeUpdateInterval = 0.25f;

	/** @brief 위젯이 생성될 때 오디오 캡처를 자동으로 시작할지 여부입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Audio")
	bool bAutoStartAudioCapture = true;

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
	/** @brief 오디오 분석기 인스턴스입니다. */
	TSharedPtr<class FStateAudioAnalyzer> AudioAnalyzer;

	/** @brief 시간 업데이트를 위한 타이머 핸들입니다. */
	FTimerHandle TimeUpdateTimerHandle;
    
	/** @brief 현재 스펙트럼 표시 값입니다. */
	float SpectrumDisplayValue = 0.f;
};