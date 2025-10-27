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

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UWidgetSwitcher> widgetSwitcher;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> Btn_Host;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> Btn_Find;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> Btn_GoHost;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> Btn_GoFind;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableText> editSessionName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableText> editSessionSize;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class USessionInfoWidget> sessionInfoWidget;

	UPROPERTY(meta=(BindWidget))
	class UScrollBox* scrollSessionList;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> textFinding;
	
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
	
	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnFindButtonClicked();

	// 세션 생성 화면 이동 함수
	UFUNCTION()
	void OnClickGoHost();
	// 세션 조회 화면 이동 함수
	UFUNCTION()
	void OnClickGoFind();

	// ========================================
	// Broadcast Event Handlers
	// ========================================

	UFUNCTION()
	void OnFindComplete(int32 idx, FString sessionName);

	/// @brief 세션 호스트 생성 이벤트 핸들러
	UFUNCTION()
	void OnSessionHost(const FString& InMapName);

	/// @brief 세션 참가 이벤트 핸들러
	UFUNCTION()
	void OnSessionJoin(const FString& Address, int32 InPort);

	/// @brief 세션 오류 이벤트 핸들러
	UFUNCTION()
	void OnSessionError(const FString& ErrorMessage);
	
	UFUNCTION()
	void SetFindingText(const FString& NewText);

private:
	/// @brief PlayerController 참조 캐싱
	TObjectPtr<class APlayerControl> CachedPlayerController;
};

	