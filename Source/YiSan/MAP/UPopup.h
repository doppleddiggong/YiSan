// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EBuildingType.h"
#include "UPopup.generated.h"

// enum class EBuildingType : uint8
// {
// 	None,
// 	
// 	Yeomingak      		UMETA(DisplayName = "여민각"),
// 	Bongsudang     		UMETA(DisplayName = "봉수당"),
// 	Naknamhyeon    		UMETA(DisplayName = "낙남헌"),
// 	Yuyotaek       		UMETA(DisplayName = "유여택"),
// 	Jwaikmun      		UMETA(DisplayName = "좌익문"),
// 	Uhwagwan      		UMETA(DisplayName = "우화관"),
// 	Byeolju       		UMETA(DisplayName = "별주"),
// 	Bijangcheol   		UMETA(DisplayName = "비장청"),
// 	Seoricheong   		UMETA(DisplayName = "서리청"),
// 	Namgunyeong   		UMETA(DisplayName = "남군영"),
// 	Sinpungnu     		UMETA(DisplayName = "신풍루"),
// 	Jibsacheong   		UMETA(DisplayName = "집사청"),
// 	Bukgunyeong   		UMETA(DisplayName = "북군영"),
// 	Eojeong       		UMETA(DisplayName = "어정"),
// 	Iancheong     		UMETA(DisplayName = "이안청"),
// 	Hyangdaecheong		UMETA(DisplayName = "행대청"),
// 	Unhangak			UMETA(DisplayName = "운한각"),
// 	Jaesil				UMETA(DisplayName = "재실"),
//
// 	Max				UMETA(Hidden)
//
//};

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingEvent, EBuildingType, BuildingType);

UCLASS()
class YISAN_API UUPopup : public UObject
{
	GENERATED_BODY()

public:
	static UUPopup* Get(UWorld* World);

	// 다른 객체들이 구독할 수 있는 이벤트
	UPROPERTY(BlueprintAssignable, Category="Broadcast")
	FOnBuildingEvent OnBuildingEvent;

	// 브로드캐스트 실행 함수
	UFUNCTION(BlueprintCallable, Category="Broadcast")
	void BroadcastBuildingEvent(EBuildingType BuildingType);
};
