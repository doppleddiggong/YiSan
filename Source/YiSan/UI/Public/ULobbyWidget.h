// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once

#include "CoreMinimal.h"
#include "UYiSanGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "ULobbyWidget.generated.h"

/// @file ULobbyWidget.h
/// @brief 멀티플레이 로비 UI 위젯을 선언합니다.

/// @brief 멀티플레이 로비 화면 위젯입니다.
/// @details Host/Join 버튼, IP 입력 필드, 연결 상태 표시 등을 제공합니다.
UCLASS()
class YISAN_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	// ========================================
	// UI Bindings (Blueprint에서 바인딩할 위젯들)
	// ========================================

	/// @brief Host 버튼 (Blueprint에서 바인딩)
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> Btn_Host;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> Btn_Find;

	/// @brief Join 버튼 (Blueprint에서 바인딩)
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> Btn_Join;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableText> editSessionName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableText> editSessionSize;

	// /// @brief Disconnect 버튼 (Blueprint에서 바인딩)
	// UPROPERTY(meta=(BindWidget))
	// TObjectPtr<class UButton> Btn_Disconnect;
	//
	// /// @brief IP 주소 입력 텍스트 박스 (Blueprint에서 바인딩)
	// UPROPERTY(meta=(BindWidget))
	// TObjectPtr<class UEditableTextBox> TxtBox_IPAddress;

	/// @brief 상태 표시 텍스트 (Blueprint에서 바인딩)
	//UPROPERTY(meta=(BindWidget))
	//TObjectPtr<class UTextBlock> Txt_Status;

	// ========================================
	// Settings
	// ========================================

	/// @brief 호스팅할 맵 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby|Settings")
	FString MapName = TEXT("MainLevel_WP");

	/// @brief 최대 플레이어 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby|Settings")
	int32 MaxPlayers = 4;

	/// @brief 서버 포트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby|Settings")
	int32 Port = 7777;

	// ========================================
	// Button Handlers
	// ========================================

	/// @brief Host 버튼 클릭 핸들러
	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnFindButtonClicked();

	// /// @brief Join 버튼 클릭 핸들러
	// UFUNCTION()
	// void OnJoinButtonClicked();

	// /// @brief Disconnect 버튼 클릭 핸들러
	// UFUNCTION()
	// void OnDisconnectButtonClicked();

	// ========================================
	// Broadcast Event Handlers
	// ========================================

	/// @brief 세션 호스트 생성 이벤트 핸들러
	UFUNCTION()
	void OnSessionHost(const FString& InMapName);

	/// @brief 세션 참가 이벤트 핸들러
	UFUNCTION()
	void OnSessionJoin(const FString& Address, int32 InPort);

	// /// @brief 세션 연결 해제 이벤트 핸들러
	// UFUNCTION()
	// void OnSessionDisconnect();

	/// @brief 세션 오류 이벤트 핸들러
	UFUNCTION()
	void OnSessionError(const FString& ErrorMessage);

	// ========================================
	// UI Update
	// ========================================

	/// @brief 상태 텍스트를 업데이트합니다.
	/// @param StatusText 표시할 상태 텍스트
	UFUNCTION(BlueprintCallable, Category="Lobby|UI")
	void UpdateStatusText(const FString& StatusText);

private:
	/// @brief PlayerController 참조 캐싱
	TObjectPtr<class APlayerControl> CachedPlayerController;
	
};

	