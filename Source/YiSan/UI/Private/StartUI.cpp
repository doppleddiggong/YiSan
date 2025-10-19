#include "StartUI.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "YiSanGameInstance.h" 
#include "Engine/Texture.h"

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
	if (MediaPlayer && MediaPlayer->IsPlaying())
	{
		MediaPlayer->Pause();
	}

	// 맵 이름이 유효하면 레벨 전환
	if (!MapName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, MapName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMapName is not set in StartUI"));
	}
	UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GetGameInstance());
	//인스턴스를 가져오자
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("GameInstance is not UYiSanGameInstance!"));
		return;
	}

	// 목료 레벨 이름 저장
	GI->TargetLevel = FName("MainMap"); 

	// 이제 mapname 에서 설정한 맵으로 이동
	if (!MapName.IsNone()) 
	{
		UGameplayStatics::OpenLevel(this, MapName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MapName (to LoadingMap) is not set in StartUI"));
	}
}