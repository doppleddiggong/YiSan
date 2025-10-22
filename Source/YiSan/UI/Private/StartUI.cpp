#include "StartUI.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "YiSanGameInstance.h" 
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

	// 서버(Host)인지 확인
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("UStartUI - World is NULL!"));
		return;
	}

	ENetMode NetMode = World->GetNetMode();

	// Host (리슨 서버 또는 데디케이티드 서버)만 레벨 전환 가능
	if (NetMode == NM_ListenServer || NetMode == NM_DedicatedServer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UStartUI - Host detected. Starting ServerTravel to MainMap_WP..."));

		// ServerTravel을 사용하면 모든 클라이언트가 자동으로 따라옴
		World->ServerTravel(TEXT("/Game/CustomContents/Maps/MainMap_WP?listen"));
	}
	else if (NetMode == NM_Client)
	{
		// 클라이언트는 시작할 수 없음
		UE_LOG(LogTemp, Warning, TEXT("UStartUI - Client cannot start the game. Waiting for Host..."));
	}
	else // NM_Standalone (싱글플레이)
	{
		// 싱글플레이는 기존 방식 사용
		UE_LOG(LogTemp, Warning, TEXT("UStartUI - Standalone mode. Using LoadLevelWithLoadingScreen..."));

		UGameInstance* GI_Raw = GetGameInstance();
		if (!GI_Raw)
		{
			UE_LOG(LogTemp, Error, TEXT("UStartUI - GetGameInstance() returned NULL!"));
			return;
		}

		UYiSanGameInstance* GI = Cast<UYiSanGameInstance>(GI_Raw);
		if (!GI)
		{
			UE_LOG(LogTemp, Error, TEXT("UStartUI - Cast to UYiSanGameInstance FAILED!"));
			return;
		}

		GI->LoadLevelWithLoadingScreen(*GameLevel::LoadingMap);
	}
}