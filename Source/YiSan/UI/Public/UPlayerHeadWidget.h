// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UPlayerHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class YISAN_API UPlayerHeadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이 위젯을 소유한 액터를 설정하고, 닉네임 확인 타이머를 시작합니다. */
	void SetOwningActor(class APlayerActor* Actor);

protected:
	virtual void NativeConstruct() override;

private:
	/** 닉네임 정보를 확인하는 함수입니다. */
	void CheckPlayerState();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> image_Icon;

	/** 위젯을 소유하고 있는 액터에 대한 약한 포인터입니다. */
	UPROPERTY()
	TWeakObjectPtr<class APlayerActor> OwningActor;

	/** 닉네임 정보를 주기적으로 확인할 타이머 핸들입니다. */
	FTimerHandle TimerHandle_CheckPlayerState;
};
