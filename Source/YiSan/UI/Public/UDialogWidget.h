// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDialogWidget.generated.h"

/// @file UDialogWidget.h
/// @brief 음성 및 텍스트 응답을 출력하는 다이얼로그 위젯을 선언합니다.
/**
 * @brief 음성 및 텍스트 응답을 출력하는 다이얼로그 위젯입니다.
 * @details 화면에 잠시 나타났다가 사라지는 토스트 메시지나 간단한 알림을 표시하는 데 사용됩니다.
 */
UCLASS()
class YISAN_API UDialogWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /// @brief 위젯 초기화 및 기본 색상을 설정합니다.
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /// @brief 대화 텍스트를 표시하고 강조 색상을 적용합니다.
    /// @param InString [in] 출력할 대화 문자열입니다.
    UFUNCTION(BlueprintCallable, Category ="Chat")
    void ShowDialog(FString InString);

    void HideDialogImmediately();

private:
    void HandleHideTimerExpired();
    
public:
    UPROPERTY(meta = (BindWidget))
    class UBorder* DialogBorder;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DialogText;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Chat")
    FLinearColor ActivateColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Chat")
    FLinearColor DeactivateColor;

private:
    /// @brief Delay 태스크 핸들 (이전 태스크 취소용)
    FTimerHandle HideTimerHandle;
};
