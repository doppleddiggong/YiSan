#include "StartUI.h"

#include "GameLogging.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "APlayerControl.h"
#include "Engine/Texture.h"
#include "ULoadingCircleManager.h"
#include "UNetworkGameInstanceSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Font.h"
#include "YiSanPlayerListManager.h"
#include "Kismet/GameplayStatics.h"

void UStartUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 레벨 로드 완료 - 로딩 화면 숨기기
	//ULoadingCircleManager::Decrease(this);

	// 버튼 클릭 이벤트 바인딩
	if (StartButton)
		StartButton->OnClicked.AddDynamic(this, &UStartUI::OnStartButtonClicked);

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

	// 마우스 보여라
	if (auto PC = GetWorld()->GetFirstPlayerController() )
	{
		// 입력 모드를 'UI 우선'으로 설정 (버튼 클릭 가능하게)
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputModeData);

		// 마우스 커서를 표시
		PC->SetShowMouseCursor(true);
	}
	
	// Find the player list manager and bind to its update delegate
	AYiSanPlayerListManager* PlayerListManager = Cast<AYiSanPlayerListManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AYiSanPlayerListManager::StaticClass()));
    if (PlayerListManager)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("UI found the PlayerListManager!"));
        PlayerListManager->OnPlayerListUpdated.AddUObject(this, &UStartUI::OnPlayerListUpdated);
        // Also, immediately update the list with the current data
        OnPlayerListUpdated(PlayerListManager->GetPlayerList());
    }
    else
    { GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("UI FAILED to find the PlayerListManager!"));
    }
}

void UStartUI::OnStartButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("UStartUI::OnStartButtonClicked - Button Clicked!"));

	if (MediaPlayer && MediaPlayer->IsPlaying())
	{
		MediaPlayer->Pause();
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UStartUI - GetOwningPlayer() returned nullptr!"));
		return;
	}

	APlayerControl* pc = Cast<APlayerControl>(PC);
	if (!pc)
	{
		UE_LOG(LogTemp, Error, TEXT("UStartUI - PlayerController is not AYourPlayerController!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UStartUI - Cast successful. Calling ServerStartMapTravel..."));

	//서버에서만 실행되도록 체크
	if (pc->HasAuthority())
	{
		// 서버(호스트)인 경우: 직접 호출
		pc->ServerStartMapTravel(TEXT("/Game/CustomContents/Maps/MainMap_WP"));
		PRINTLOG(TEXT("로딩 레벨 매니저: 서버가 맵 전환 시작"));
	}
	else
	{
		// 클라이언트인 경우: 서버에게 요청
		// Server RPC가 필요함 (아래 추가 코드 참고)
		pc->Server_RequestMapTravel(TEXT("/Game/CustomContents/Maps/MainMap_WP"));
		PRINTLOG(TEXT("로딩 레벨 매니저: 클라이언트가 서버에 맵 전환 요청"));
	}
}
	
void UStartUI::UpdatePlayerList(const TArray<FString>& playerNames)
{
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

UTextBlock* UStartUI::CreatePlayerText(const FString& playerName)
{
    UTextBlock* newText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (newText)
	{
		newText->SetText(FText::FromString(playerName));
		newText->SetJustification(ETextJustify::Right);
		newText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

		// 폰트 적용
		UFont* fontObj = LoadObject<UFont>(nullptr, TEXT("/Game/CustomContents/UI/Fonts/NotoSerifKR-Regular_Font.NotoSerifKR-Regular_Font.NotoSerifKR-Regular_Font"));
		if (fontObj)
		{
			FSlateFontInfo fontInfo;
			fontInfo.FontObject = fontObj;
			fontInfo.Size = 24; // 원하는 크기
			newText->SetFont(fontInfo);
		}
	}
	return newText;
}

void UStartUI::OnPlayerListUpdated(const TArray<FString>& NewPlayerList)
{
    UpdatePlayerList(NewPlayerList);
}
