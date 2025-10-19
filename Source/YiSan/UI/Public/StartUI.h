#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartUI.generated.h"

// 전방 선언
class UButton;
class UImage;
class UMediaPlayer;
class UMediaTexture;

UCLASS()
class YISAN_API UStartUI : public UUserWidget
{
    GENERATED_BODY()

protected:
    // UUserWidget의 생성자 역할을 하는 함수입니다.
    virtual void NativeConstruct() override;

public:
    // 시작하기 버튼
    UPROPERTY(meta = (BindWidget))
    UButton* StartButton;

    // 영상이 출력될 이미지 위젯 
    UPROPERTY(meta = (BindWidget))
    UImage* BackgroundVideoImage;

    // 인트로 영상 재생을 위한 미디어 플레이어 (에디터에서 할당 필요) = 오류 블루프린트 에서 제어
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    UMediaPlayer* MediaPlayer;

    // 미디어 플레이어의 출력을 담을 텍스처 (에디터에서 할당 필요) = 오류 블루프린트 에서 제어
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    UMediaTexture* MediaTexture;

    // 미디어 플레이어의 이미지를 메테리얼로 할당하여 적용  = 오류 블루프린트 에서 제어
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    class UMaterialInstance* introMtl;
    // '시작하기' 버튼 클릭 시 호출될 함수
    UFUNCTION()
    void OnStartButtonClicked();

    // 전환될 메인 맵의 이름 (에디터에서 설정)
    UPROPERTY(EditAnywhere, Category = "Intro Settings")
    FName MainMapName = "MainMap_WP";
};