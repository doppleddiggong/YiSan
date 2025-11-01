#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "UStartWidget.generated.h"

/// @file StartUI.h
/// @brief 인트로 영상을 재생하고 게임 시작을 안내하는 스타트 UI를 선언합니다.

/// @brief 시작 버튼과 배경 영상을 제어하는 초기 진입 위젯입니다.
UCLASS()
class YISAN_API UStartWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    /// @brief 위젯이 생성될 때 미디어 재생 설정을 초기화합니다.
    virtual void NativeConstruct() override;

private:
    /// @brief 시작 버튼 클릭 시 호출됩니다.
    UFUNCTION()
    void OnStartButtonClicked();
    
public:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<class UImage> Image_Square_0;
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<class UImage> Image_Square_1;
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<class UImage> Image_Square_2;
    
    /// @brief 시작 버튼 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> StartButton;

    /// @brief 인트로 영상이 표시되는 이미지 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UImage> BackgroundVideoImage;
    /// @brief 인트로 영상을 재생할 미디어 플레이어입니다.
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    TObjectPtr<class UMediaPlayer> MediaPlayer;
    /// @brief 미디어 플레이어 출력을 표시할 텍스처입니다.
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    TObjectPtr<class UMediaTexture> MediaTexture;
    /// @brief 영상 텍스처를 적용할 머티리얼 인스턴스입니다.
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    TObjectPtr<class UMaterialInstance> introMtl;

    
    bool bSuccessd = false;


    //=========참여자 명단 가져오기=========
    UFUNCTION(BlueprintCallable)
    void UpdatePlayerList(const TArray<FString>& playerNames);

    UFUNCTION()
    UTextBlock* CreatePlayerText(const FString& playerName);
    void OnPlayerListUpdated(const TArray<FString>& NewPlayerList);

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<class UVerticalBox> playerList;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<class UVerticalBox> playerList;

};
