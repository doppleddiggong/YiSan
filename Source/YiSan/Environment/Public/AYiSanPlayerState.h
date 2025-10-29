// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AYiSanPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class YISAN_API AYiSanPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AYiSanPlayerState();

	/** 닉네임 (로비에서 입력한 이름) */
	UPROPERTY(ReplicatedUsing = OnRep_Nickname, BlueprintReadOnly, Category="Player Info")
	FString Nickname;

	/** 입장 순서 인덱스 (0,1,2,3...) */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex, BlueprintReadOnly, Category="Player Info")
	int32 PlayerIndex;

	/** 서버에서 닉네임/인덱스 세팅 */
	void SetPlayerInfo(const FString& InName, int32 InIndex);

protected:
	UFUNCTION()
	void OnRep_Nickname();

	UFUNCTION()
	void OnRep_PlayerIndex();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
