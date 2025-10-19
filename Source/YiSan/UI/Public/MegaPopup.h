// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MegaPopup.generated.h"

/**
 * 
 */
UCLASS()
class YISAN_API UMegaPopup : public UUserWidget
{
	GENERATED_BODY()

	void NativeConstruct();
	
public:
	// GPT 응답 텍스트 표시용
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Description;
	// GPT 응답을 화면에 표시
	UFUNCTION(BlueprintCallable, Category = "MegaPopup")
	void SetDescription(const FString& InText);
	// 로딩 메시지 표시/숨김 (사용 여부 고려)
	UFUNCTION(BlueprintCallable, Category = "MegaPopup")
	void ShowLoading(bool bShow); 
};
