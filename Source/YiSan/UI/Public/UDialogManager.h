// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UDialogManager.generated.h"

UCLASS()
class YISAN_API UDialogManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_LOCALPLAYER_FROM_PC_SUBSYSTEM_GETTER_INLINE(UDialogManager);
	DEFINE_LOCALPLAYER_SUBSYSTEM_GETTER_INLINE(UDialogManager);

	UDialogManager();

	// 다이얼로그 요청 함수
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void ShowToast(const FString& Message);

protected:
	/** @brief 현재 월드에 위젯이 없으면 생성하고 뷰포트에 부착합니다. */
	void EnsureWidgetForWorld(UWorld* World);

protected:
	UPROPERTY()
	TSubclassOf<class UDialogWidget> DialogWidgetClass;

	UPROPERTY()
	TObjectPtr<class UDialogWidget> DialogWidget;
};
