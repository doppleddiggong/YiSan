#include "UStartWidget.h"

#include "GameLogging.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "APlayerControl.h"
#include "AYisanGameState.h"
#include "Engine/Texture.h"
#include "Blueprint/WidgetTree.h"
#include "UNetworkGameInstanceSubsystem.h" // Added include

void UStartWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	StartButton->OnClicked.AddDynamic(this, &UStartWidget::OnStartButtonClicked);

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
		PRINTLOG(TEXT("MediaPlayer, MediaTexture, or BackgroundVideoImage not set in StartUI"));
	}

	// 마우스 보여라
	if (auto PC = GetOwningPlayer() )
	{
		// 입력 모드를 'UI 우선'으로 설정 (버튼 클릭 가능하게)
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputModeData);
	
		// 마우스 커서를 표시
		PC->SetShowMouseCursor(true);
	
		StartButton->SetVisibility(PC->HasAuthority() ? ESlateVisibility::Visible :  ESlateVisibility::Hidden );
		Image_Square_0->SetVisibility(PC->HasAuthority() ? ESlateVisibility::Visible :  ESlateVisibility::Hidden );
		Image_Square_1->SetVisibility(PC->HasAuthority() ? ESlateVisibility::Visible :  ESlateVisibility::Hidden );
		Image_Square_2->SetVisibility(PC->HasAuthority() ? ESlateVisibility::Visible :  ESlateVisibility::Hidden );
	}
	
	if (!playerList)
	{
		PRINTLOG(TEXT("UStartUI::playerList is NULL! Check WBP_StartUI binding."));
	}
	
	// Get the Network Game Instance Subsystem
	if (auto NetworkSubsystem = UNetworkGameInstanceSubsystem::Get(GetWorld()) )
	{
		NetworkSubsystem->OnPlayerListUpdated.AddUObject(this, &UStartWidget::OnPlayerListUpdated);
		NetworkSubsystem->RequestPlayerListRefresh();
	}
	else
	{
		PRINTLOG(TEXT("NetworkSubsystem is nullptr"));
	}
}

void UStartWidget::OnStartButtonClicked()
{
	PRINTLOG( TEXT("UStartUI::OnStartButtonClicked - Button Clicked!"));

	if (MediaPlayer && MediaPlayer->IsPlaying())
		MediaPlayer->Pause();

	APlayerControl* PC = nullptr;
	if ( APlayerController* Ctrl = GetOwningPlayer() )
	{
		PC = Cast<APlayerControl>(Ctrl);
	}
	else
	{
		PRINTLOG(TEXT("UStartUI - GetOwningPlayer() is nullptr"));
		return;
	}

	if (PC == nullptr)
	{
		PRINTLOG( TEXT("UStartUI - APlayerControl is nullptr"));
		return;
	}

	PRINTLOG( TEXT("UStartUI - Cast successful. Calling ServerStartMapTravel..."));

	//서버에서만 실행되도록 체크
	if (PC->HasAuthority())
	{
		// 서버(호스트)인 경우: 직접 호출
		PC->ServerStartMapTravel(TEXT("/Game/CustomContents/Maps/MainMap_WP"));
		PRINTLOG(TEXT("로딩 레벨 매니저: 서버가 맵 전환 시작"));

		StartButton->SetVisibility(ESlateVisibility::Hidden);
		
		if (auto GS = GetWorld()->GetGameState<AYisanGameState>())
			GS->MulticastRPC_ToastMessage(TEXT("화성 투어를 시작합니다"));
	}
	else
	{
		// 클라이언트인 경우: 서버에게 요청
		// Server RPC가 필요함 (아래 추가 코드 참고)
		PC->Server_RequestMapTravel(TEXT("/Game/CustomContents/Maps/MainMap_WP"));
		PRINTLOG(TEXT("로딩 레벨 매니저: 클라이언트가 서버에 맵 전환 요청"));
	}
}
	
void UStartWidget::UpdatePlayerList(const TArray<FString>& playerNames)
{
    PRINTLOG(TEXT("UStartUI::UpdatePlayerList called with %d player(s)."), playerNames.Num());
	if (!playerList) return;

	playerList->ClearChildren(); // Clear existing entries
	playerList->SetVisibility(ESlateVisibility::Visible);

	for (const FString& Name : playerNames)
	{
		UTextBlock* PlayerText = CreatePlayerText(Name);
		if (PlayerText)
		{
			playerList->AddChild(PlayerText);
		}
	}
}

UTextBlock* UStartWidget::CreatePlayerText(const FString& playerName)
{
    PRINTLOG(TEXT("UStartUI::CreatePlayerText creating text for: %s"), *playerName);
	
    UTextBlock* newText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (newText)
	{
		newText->SetText(FText::FromString(playerName));
		newText->SetJustification(ETextJustify::Right);
		newText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
	return newText;
}

void UStartWidget::OnPlayerListUpdated(const TArray<FString>& NewPlayerList)
{
    UpdatePlayerList(NewPlayerList);
}


