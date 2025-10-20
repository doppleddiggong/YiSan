// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ULobbyWidget.h"
#include "APlayerControl.h"
#include "UBroadcastManager.h"
#include "GameLogging.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
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
	{
		Btn_Host->OnClicked.AddDynamic(this, &ULobbyWidget::OnHostButtonClicked);
	}

	if (Btn_Join)
	{
		Btn_Join->OnClicked.AddDynamic(this, &ULobbyWidget::OnJoinButtonClicked);
	}

	if (Btn_Disconnect)
	{
		Btn_Disconnect->OnClicked.AddDynamic(this, &ULobbyWidget::OnDisconnectButtonClicked);
		Btn_Disconnect->SetVisibility(ESlateVisibility::Collapsed); // 초기에는 숨김
	}

	// BroadcastManager 이벤트 바인딩
	if (auto BroadcastManager = UBroadcastManager::Get(GetWorld()))
	{
		BroadcastManager->OnSessionHost.AddDynamic(this, &ULobbyWidget::OnSessionHost);
		BroadcastManager->OnSessionJoin.AddDynamic(this, &ULobbyWidget::OnSessionJoin);
		BroadcastManager->OnSessionDisconnect.AddDynamic(this, &ULobbyWidget::OnSessionDisconnect);
		BroadcastManager->OnSessionError.AddDynamic(this, &ULobbyWidget::OnSessionError);
	}

	// 초기 상태 메시지
	UpdateStatusText(TEXT("로비 - Host 또는 Join을 선택하세요"));

	PRINTLOG(TEXT("[LobbyWidget] NativeConstruct - Widget initialized"));
}

void ULobbyWidget::NativeDestruct()
{
	// BroadcastManager 이벤트 언바인딩
	if (auto BroadcastManager = UBroadcastManager::Get(GetWorld()))
	{
		BroadcastManager->OnSessionHost.RemoveDynamic(this, &ULobbyWidget::OnSessionHost);
		BroadcastManager->OnSessionJoin.RemoveDynamic(this, &ULobbyWidget::OnSessionJoin);
		BroadcastManager->OnSessionDisconnect.RemoveDynamic(this, &ULobbyWidget::OnSessionDisconnect);
		BroadcastManager->OnSessionError.RemoveDynamic(this, &ULobbyWidget::OnSessionError);
	}

	Super::NativeDestruct();

	PRINTLOG(TEXT("[LobbyWidget] NativeDestruct - Widget destroyed"));
}

// ========================================
// Button Handlers
// ========================================

void ULobbyWidget::OnHostButtonClicked()
{
	PRINTLOG(TEXT("[LobbyWidget] OnHostButtonClicked - Map=%s, MaxPlayers=%d"), *MapName, MaxPlayers);

	if (CachedPlayerController)
	{
		CachedPlayerController->HostSession(MapName, MaxPlayers);
		UpdateStatusText(FString::Printf(TEXT("호스트 생성 중... 맵: %s"), *MapName));
	}
	else
	{
		PRINTLOG(TEXT("[LobbyWidget] OnHostButtonClicked - PlayerController is null"));
		UpdateStatusText(TEXT("오류: PlayerController를 찾을 수 없습니다"));
	}
}

void ULobbyWidget::OnJoinButtonClicked()
{
	if (!TxtBox_IPAddress)
	{
		PRINTLOG(TEXT("[LobbyWidget] OnJoinButtonClicked - IP TextBox is null"));
		UpdateStatusText(TEXT("오류: IP 입력 필드를 찾을 수 없습니다"));
		return;
	}

	FString IPAddress = TxtBox_IPAddress->GetText().ToString();

	if (IPAddress.IsEmpty())
	{
		PRINTLOG(TEXT("[LobbyWidget] OnJoinButtonClicked - IP Address is empty"));
		UpdateStatusText(TEXT("오류: IP 주소를 입력하세요"));
		return;
	}

	PRINTLOG(TEXT("[LobbyWidget] OnJoinButtonClicked - Address=%s, Port=%d"), *IPAddress, Port);

	if (CachedPlayerController)
	{
		CachedPlayerController->JoinSession(IPAddress, Port);
		UpdateStatusText(FString::Printf(TEXT("서버 접속 중... %s:%d"), *IPAddress, Port));
	}
	else
	{
		PRINTLOG(TEXT("[LobbyWidget] OnJoinButtonClicked - PlayerController is null"));
		UpdateStatusText(TEXT("오류: PlayerController를 찾을 수 없습니다"));
	}
}

void ULobbyWidget::OnDisconnectButtonClicked()
{
	PRINTLOG(TEXT("[LobbyWidget] OnDisconnectButtonClicked"));

	if (CachedPlayerController)
	{
		CachedPlayerController->DisconnectSession();
		UpdateStatusText(TEXT("연결 해제 중..."));
	}
	else
	{
		PRINTLOG(TEXT("[LobbyWidget] OnDisconnectButtonClicked - PlayerController is null"));
		UpdateStatusText(TEXT("오류: PlayerController를 찾을 수 없습니다"));
	}
}

// ========================================
// Broadcast Event Handlers
// ========================================

void ULobbyWidget::OnSessionHost(const FString& InMapName)
{
	PRINTLOG(TEXT("[LobbyWidget] OnSessionHost - Map=%s"), *InMapName);
	UpdateStatusText(FString::Printf(TEXT("호스트 생성 완료! 맵: %s (난입 허용)"), *InMapName));

	// Disconnect 버튼 표시
	if (Btn_Disconnect)
	{
		Btn_Disconnect->SetVisibility(ESlateVisibility::Visible);
	}

	// Host/Join 버튼 숨김
	if (Btn_Host)
	{
		Btn_Host->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Btn_Join)
	{
		Btn_Join->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ULobbyWidget::OnSessionJoin(const FString& Address, int32 InPort)
{
	PRINTLOG(TEXT("[LobbyWidget] OnSessionJoin - Address=%s, Port=%d"), *Address, InPort);
	UpdateStatusText(FString::Printf(TEXT("서버 접속 완료! %s:%d"), *Address, InPort));

	// Disconnect 버튼 표시
	if (Btn_Disconnect)
	{
		Btn_Disconnect->SetVisibility(ESlateVisibility::Visible);
	}

	// Host/Join 버튼 숨김
	if (Btn_Host)
	{
		Btn_Host->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Btn_Join)
	{
		Btn_Join->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ULobbyWidget::OnSessionDisconnect()
{
	PRINTLOG(TEXT("[LobbyWidget] OnSessionDisconnect"));
	UpdateStatusText(TEXT("연결 해제됨 - Host 또는 Join을 선택하세요"));

	// Disconnect 버튼 숨김
	if (Btn_Disconnect)
	{
		Btn_Disconnect->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Host/Join 버튼 표시
	if (Btn_Host)
	{
		Btn_Host->SetVisibility(ESlateVisibility::Visible);
	}

	if (Btn_Join)
	{
		Btn_Join->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULobbyWidget::OnSessionError(const FString& ErrorMessage)
{
	PRINTLOG(TEXT("[LobbyWidget] OnSessionError - Error=%s"), *ErrorMessage);
	UpdateStatusText(FString::Printf(TEXT("오류: %s"), *ErrorMessage));
}

// ========================================
// UI Update
// ========================================

void ULobbyWidget::UpdateStatusText(const FString& StatusText)
{
	if (Txt_Status)
	{
		Txt_Status->SetText(FText::FromString(StatusText));
	}
}
