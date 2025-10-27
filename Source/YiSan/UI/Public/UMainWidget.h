// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Blueprint/UserWidget.h"
#include "EndingWidget.h" // EndingWidget 헤더 추가
#include "UMainWidget.generated.h"

/// @file UMainWidget.h
/// @brief 음성 상호작용과 건물 안내 UI를 제공하는 메인 위젯을 선언합니다.
/// @brief 채팅, 팝업, 네트워크 응답을 중재하는 게임 메인 UI 위젯입니다.

UCLASS()
class YISAN_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UMainWidget(const FObjectInitializer& ObjectInitializer);

protected:
	/// @brief 위젯 초기화와 브로드캐스트 구독을 수행합니다.
	virtual void NativeConstruct() override;
	void PlayBGM();

private:
	UFUNCTION()
	void OnNearBuilding(EBuildingType InBuildingType);
    UFUNCTION()
    void OnMegaPopupClosed();

    bool IsMegaPopupVisible() const;
    bool IsSmallPopupVisible() const;
public:
    
    /// @brief 메가 팝업이 배치되는 컨테이너 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    class UMegaPopup* MegaPopupCtn;
    
    /// @brief 스몰 팝업이 배치되는 컨테이너 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    class USmallPopup* SmallPopupCtn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BGM")
    class USoundBase* BGM_Sound;
    UPROPERTY()
    class UAudioComponent* BGM_AudioComp;


    // 엔딩 위젯
    UPROPERTY(EditAnywhere, Category = "Widgets")
    TSubclassOf<UEndingWidget> EndingWidgetClass;
    
    // 생성된 엔딩 위젯의 인스턴스를 저장할 변수
    UPROPERTY()
    UEndingWidget* EndingWidgetInstance;
    
    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager;
    
    /// @brief 최근 감지된 건물 유형입니다.
    EBuildingType CurNearBuildingType = EBuildingType::None;
};

