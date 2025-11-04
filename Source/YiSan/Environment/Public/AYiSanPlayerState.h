// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AYiSanPlayerState.generated.h"

/**
 * @file AYiSanPlayerState.h
 * @brief AYiSanPlayerState 클래스를 선언합니다.
 */

/**
 * @brief @c APlayerState를 확장해 닉네임과 플레이어 인덱스를 복제합니다.
 */
UCLASS()
class YISAN_API AYiSanPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
        AYiSanPlayerState();
        virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

        /** @brief 권한 닉네임 값을 갱신합니다. */
        void SetPlayerInfo(const FString& InName);
        /** @brief UI 정렬에 사용할 권한 플레이어 인덱스를 설정합니다. */
        void SetPlayerIndex(const int32 InPlayerIndex);

        /** @brief 플레이어 인덱스를 기반으로 아이콘 경로를 결정합니다. */
        FString GetResourcePath()
	{
		switch (PlayerIndex)
		{
		case 0:
			return TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_red");
		case 1:
			return TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_green");
		case 2:
			return TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_blue");
		case 3:
			return TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_black");
		case 4:
			return TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_white");
		default:
			return TEXT("/Game/CustomContents/UI/Texture/icon_yisan512_white");
		}
	}	
	
protected:
        /** @brief 닉네임이 클라이언트에서 변경될 때 호출됩니다. */
        UFUNCTION()
        void OnRep_Nickname();

        /** @brief 플레이어 인덱스가 클라이언트에서 갱신될 때 호출됩니다. */
        UFUNCTION()
        void OnRep_PlayerIndex();

public:
	/** 닉네임 (로비에서 입력한 이름) */
	UPROPERTY(ReplicatedUsing = OnRep_Nickname, BlueprintReadOnly, Category="Player Info")
	FString Nickname;

	/** 입장 순서 인덱스 (0,1,2,3...) */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex, BlueprintReadOnly, Category="Player Info")
	int32 PlayerIndex;
};
