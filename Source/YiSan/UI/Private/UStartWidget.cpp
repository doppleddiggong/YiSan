#include "UStartWidget.h"

#include "GameLogging.h"
#include "Components/Button.h"
#include "Components/Image.h"
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
	
	if (auto NetworkSubsystem = UNetworkGameInstanceSubsystem::Get(GetWorld()) )
	{
		NetworkSubsystem->OnPlayerListUpdated.AddUObject(this, &UStartWidget::OnPlayerListUpdated);
		NetworkSubsystem->RequestPlayerListRefresh();
	}
	else
	{
		PRINTLOG(TEXT("UStartWidget::NativeConstruct - NetworkSubsystem is nullptr!"));
	}
}

void UStartWidget::OnStartButtonClicked()
{
	PRINTLOG( TEXT("UStartUI::OnStartButtonClicked - Button Clicked!"));

	// if (MediaPlayer && MediaPlayer->IsPlaying())
	// 	MediaPlayer->Pause();

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

	// // 내 PlayerIndex 구하기
	// int32 LocalPlayerIndex = -1;
	// if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	// {
	// 	if (AYiSanPlayerState* LocalPS = PC->GetPlayerState<AYiSanPlayerState>())
	// 	{
	// 		LocalPlayerIndex = LocalPS->PlayerIndex;
	// 		PRINTLOG(TEXT("LocalPlayerIndex = %d"), LocalPlayerIndex);
	// 	}
	// }

	
	// 내 PlayerIndex 구하기 - 분할 화면/멀티플레이 모두 지원되도록 Owning Player 기준으로 조회한다.
	int32 LocalPlayerIndex = INDEX_NONE;
	const AYiSanPlayerState* LocalPlayerState = GetOwningPlayerState<AYiSanPlayerState>();

	if (!LocalPlayerState)
	{
		if (const APlayerController* PlayerController = GetOwningPlayer())
		{
			LocalPlayerState = PlayerController->GetPlayerState<AYiSanPlayerState>();
		}
	}

	if (LocalPlayerState)
	{
		LocalPlayerIndex = LocalPlayerState->PlayerIndex;
		PRINTLOG(TEXT("LocalPlayerIndex = %d"), LocalPlayerIndex);
	}
	else
	{
		PRINTLOG(TEXT("Failed to resolve LocalPlayerIndex. Using INDEX_NONE."));
	}
	
	for (const FString& PlayerInfoString : playerNames)
	{
		TArray<FString> PlayerInfo;
		PlayerInfoString.ParseIntoArray(PlayerInfo, TEXT(":"), true);

		if (PlayerInfo.Num() == 2)
		{
			int32 PlayerIndex = FCString::Atoi(*PlayerInfo[0]);
			FString PlayerName = PlayerInfo[1];

			UPlayerListItem* PlayerListItem = CreatePlayerListItem(PlayerIndex, LocalPlayerIndex, PlayerName);
			if (PlayerListItem)
			{
				playerList->AddChild(PlayerListItem);
			}
		}
	}
}

UPlayerListItem* UStartWidget::CreatePlayerListItem(const int32 InPlayerIndex, const int32 LocalPlayerIndex, const FString& InPlayerName)
{
    PRINTLOG(TEXT("UStartUI::CreatePlayerListItem creating item for: [%d](%s)"), InPlayerIndex, *InPlayerName);

    if (PlayerListItemClass)
    {
	    UPlayerListItem* NewItem = CreateWidget<UPlayerListItem>(this, PlayerListItemClass);
        if (NewItem)
        {
			NewItem->SetPlayerStatus(InPlayerIndex, LocalPlayerIndex, InPlayerName);
            return NewItem;
        }
    }
    return nullptr;
}

void UStartWidget::OnPlayerListUpdated(const TArray<FString>& NewPlayerList)
{
    PRINTLOG(TEXT("UStartWidget::OnPlayerListUpdated - Received player list with %d players"), NewPlayerList.Num());
    for (int32 i = 0; i < NewPlayerList.Num(); i++)
    {
        PRINTLOG(TEXT("UStartWidget::OnPlayerListUpdated - Player %d: %s"), i, *NewPlayerList[i]);
    }
    UpdatePlayerList(NewPlayerList);
}


