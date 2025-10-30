#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "StartUI.generated.h"

// 전방 선언
class UButton;
class UImage;
class UMediaPlayer;
class UMediaTexture;

/// @file StartUI.h
/// @brief 인트로 영상을 재생하고 게임 시작을 안내하는 스타트 UI를 선언합니다.

/// @brief 시작 버튼과 배경 영상을 제어하는 초기 진입 위젯입니다.
UCLASS()
class YISAN_API UStartUI : public UUserWidget
{
    GENERATED_BODY()

protected:
    /// @brief 위젯이 생성될 때 미디어 재생 설정을 초기화합니다.
    virtual void NativeConstruct() override;

public:
    /// @brief 시작 버튼 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    UButton* StartButton;

    /// @brief 인트로 영상이 표시되는 이미지 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    UImage* BackgroundVideoImage;

    /// @brief 인트로 영상을 재생할 미디어 플레이어입니다.
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    UMediaPlayer* MediaPlayer;

    /// @brief 미디어 플레이어 출력을 표시할 텍스처입니다.
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    UMediaTexture* MediaTexture;

    /// @brief 영상 텍스처를 적용할 머티리얼 인스턴스입니다.
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    class UMaterialInstance* introMtl;

    /// @brief 시작 버튼 클릭 시 호출됩니다.
    UFUNCTION()
    void OnStartButtonClicked();

    /// @brief 전환할 메인 맵 이름입니다.
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    FName MapName = "LoadingMap";

    bool bSucessd = false;


    //=========참여자 명단 가져오기=========
    UFUNCTION(BlueprintCallable)
    void UpdatePlayerList(const TArray<FString>& playerNames);
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UVerticalBox> playerList;
    UFUNCTION()
    UTextBlock* CreatePlayerText(const FString& playerName);
    void OnPlayerListUpdated(const TArray<FString>& NewPlayerList);
    
};
