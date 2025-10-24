#include "StartUI.h"

#include "GameLogging.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "YiSanGameInstance.h" 
#include "YiSanLoading.h"
#include "Engine/Texture.h"
#include "YiSan/YiSan.h"

void UStartUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UStartUI::OnStartButtonClicked);
	}

	// 미디어 플레이어와 텍스처가 올바르게 설정되어 있다면 영상 재생 준비
	if (MediaPlayer && MediaTexture && BackgroundVideoImage)
	{
		// 미디어 텍스처에 미디어 플레이어 연결
		MediaTexture->SetMediaPlayer(MediaPlayer);

		// 이미지 위젯에 미디어 텍스처 적용
		BackgroundVideoImage->SetBrushFromMaterial(introMtl);
		
		// 자동 재생
		MediaPlayer->Play();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MediaPlayer, MediaTexture, or BackgroundVideoImage not set in StartUI"));
	}
}

void UStartUI::OnStartButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("UStartUI::OnStartButtonClicked - Button Clicked!"));

	if (MediaPlayer && MediaPlayer->IsPlaying())
	{
		MediaPlayer->Pause();
	}

	UGameInstance* GI_Raw = GetGameInstance();
	if (!GI_Raw)
	{
		UE_LOG(LogTemp, Error, TEXT("UStartUI - GetGameInstance() returned NULL!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("UStartUI - Got GameInstance of class: %s"), *GI_Raw->GetClass()->GetName());


	UYiSanLoading* GI = Cast<UYiSanLoading>(GI_Raw);
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("UStartUI - Cast to UYiSanGameInstance FAILED!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UStartUI - Cast successful. Calling LoadLevelWithLoadingScreen..."));
	// GI->LoadLevelWithLoadingScreen(*GameLevel::LoadingMap);

	if (GI)
	{
		// 게임 인스턴스의 로딩 시퀀스 Step 1을 호출함
		GI->Step1_StartLoadingTargetLevel();
		PRINTLOG(TEXT("로딩 레벨 매니저: Step1 로딩 시작 호출함"));
	}
	else
	{
		PRINTLOG(TEXT("로딩 레벨 매니저: YiSanGameInstance를 찾을 수 없음!"));
	}

}