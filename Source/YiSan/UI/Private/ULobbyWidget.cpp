// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ULobbyWidget.h"

#include "UNetworkGameInstanceSubsystem.h"
#include "APlayerControl.h"
#include "UBroadcastManager.h"
#include "GameLogging.h"
#include "USessionInfoWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/PlayerController.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// PlayerController 캐싱
	if (APlayerController* PC = GetOwningPlayer())
	{
		CachedPlayerController = Cast<APlayerControl>(PC);
	}

	// 버튼 이벤트 바인딩
	if (Btn_Host)
	{Btn_Host->OnClicked.AddDynamic(this, &ULobbyWidget::OnHostButtonClicked);}
	if (Btn_Find)
	{Btn_Find->OnClicked.AddDynamic(this, &ULobbyWidget::OnFindButtonClicked);}
	if (Btn_GoHost)
	{Btn_GoHost->OnClicked.AddDynamic(this, &ULobbyWidget::OnClickGoHost);}
	if (Btn_GoFind)
	{Btn_GoFind->OnClicked.AddDynamic(this, &ULobbyWidget::OnClickGoFind);}
	UNetworkGameInstanceSubsystem::Get(GetWorld())->onFindComplete.BindUObject(this, &ULobbyWidget::OnFindComplete);

	PRINTLOG(TEXT("[LobbyWidget] NativeConstruct - Widget initialized"));
}

void ULobbyWidget::NativeDestruct()
{
	Super::NativeDestruct();

	PRINTLOG(TEXT("[LobbyWidget] NativeDestruct - Widget destroyed"));
}

// ========================================
// Button Handlers
// ========================================

void ULobbyWidget::OnHostButtonClicked()
{
	UTextBlock* buttonText = Cast<UTextBlock>(Btn_Host->GetChildAt(0));

	buttonText->SetText(FText::FromString(TEXT("만드는 중")));
	
	FText nameText = editSessionName->GetText();
	FText sizeText = editSessionSize->GetText();

	if (nameText.IsEmpty() || sizeText.IsEmpty()) return;
	
	FString sessionName = nameText.ToString();
	int32 sessionSize = FCString::Atoi(*(sizeText.ToString()));
	
	PRINTLOG(TEXT("[LobbyWidget] OnHostButtonClicked - Map=%s, SessionName=%s, MaxPlayers=%d"), *MapName, *sessionName, MaxPlayers);

	UNetworkGameInstanceSubsystem::Get(GetWorld())->CreateMySession(sessionName, sessionSize);
}

void ULobbyWidget::OnFindButtonClicked()
{
	UNetworkGameInstanceSubsystem::Get(GetWorld())->FindOtherSession();
	SetFindingText(TEXT("방 찾는 중..."));
}

void ULobbyWidget::OnFindComplete(int32 idx, FString sessionName)
{
	SetFindingText(TEXT("방 목록"));
	Btn_Find->SetVisibility(ESlateVisibility::Visible);
	if (sessionName.IsEmpty() == false && idx >= 0)
	{
		// sessionInfoWidget 만들자.
		USessionInfoWidget* item = CreateWidget<USessionInfoWidget>(GetWorld(), sessionInfoWidget);
		// 만들어진 item 을 scrollSessionList 에 추가
		scrollSessionList->AddChild(item);
		// 만들어지 item 정보 설정
		{item->SetSessionInfo(idx, sessionName);}
	}
}

void ULobbyWidget::OnSessionHost(const FString& InMapName)
{
	PRINTLOG(TEXT("[LobbyWidget] OnSessionHost - Map=%s"), *InMapName);
}

void ULobbyWidget::OnSessionJoin(const FString& Address, int32 InPort)
{
	PRINTLOG(TEXT("[LobbyWidget] OnSessionJoin - Address=%s, Port=%d"), *Address, InPort);
}

void ULobbyWidget::OnSessionError(const FString& ErrorMessage)
{
	PRINTLOG(TEXT("[LobbyWidget] OnSessionError - Error=%s"), *ErrorMessage);
}

void ULobbyWidget::OnClickGoHost()
{
	// 세션 생성 화면으로 이동
	widgetSwitcher->SetActiveWidgetIndex(1);
}

void ULobbyWidget::OnClickGoFind()
{
	// 세션 조회 화면으로 이동
	widgetSwitcher->SetActiveWidgetIndex(2);
	// 세션 조회
	OnFindButtonClicked();
}

void ULobbyWidget::SetFindingText(const FString& NewText)
{
	if (textFinding)
	{
		textFinding->SetText(FText::FromString(NewText));
	}
}