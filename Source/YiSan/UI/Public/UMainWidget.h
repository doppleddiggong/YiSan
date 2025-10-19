// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Blueprint/UserWidget.h"
#include "NetworkData.h"

#include "MegaPopup.h"
#include "SmallPopup.h"
#include "UMainWidget.generated.h"

class USmallPopup;
class UMegaPopup;

UCLASS()
class YISAN_API UMainWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void ToggleChatBox();

protected:
    virtual void NativeConstruct() override;
    //창 누르기 위해
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    UFUNCTION()
    void OnMessageComitted(const FText& Text, ETextCommit::Type CommitMethod);
    
private:
    void SendChatMessage(const FString& InMsg);
    FGPTContext BuildSpatialContext() const;

    UFUNCTION()
    void OnResponseAsk(FResponseAsk& Response, bool bSuccess);

protected:
    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
    class UCanvasPanel* ChatBox;
    
    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
    class UEditableTextBox* InputText;

private:
    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager;
public:

    // cpp 로 제어 안할꺼라 bind 할떄 이름 바꾸는걸로
    UPROPERTY(meta = (BindWidget))
    USmallPopup* SmallPopupCtn;
    UPROPERTY(meta = (BindWidget))
    UMegaPopup* MegaPopupCtn;
    // t 입력시 호출
    void ToggleMegaPopup();

    UFUNCTION()
    void OnNearBuildingBroadcast(EBuildingType BuildingType);

    // 블루프린트에서 구현
    UFUNCTION(BlueprintImplementableEvent)
    void BPI_UpdateSmallPopupText(EBuildingType BuildingType);

    UFUNCTION(BlueprintImplementableEvent)
    void BPI_UpdateDetailedInfo(EBuildingType BuildingType);
    
    // mega popup 상태
    bool bIsMegaPopupVisible = false;
    // 건물 타입 저장 용 (바꿀꺼)
    EBuildingType CurNearBuildingType = EBuildingType::None;
    // gpt 를 응답 처리할 함수
    UFUNCTION()
    void OnMegaPopupResponse(FResponseAsk& Response, bool bSuccess);

};

