// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "StartUI.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UNetworkGameInstanceSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FFindComplete, int32, FString);
/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerListUpdated, const TArray<FString>& /* PlayerNames */);

UCLASS()
class YISAN_API UNetworkGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	DEFINE_SUBSYSTEM_GETTER_INLINE(UNetworkGameInstanceSubsystem);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
public:
	FFindComplete onFindComplete;

	//세션의 모든 처리 진행 객체
	IOnlineSessionPtr sessionInterface;

	//세션 생성 
	//세션 생성 함수
	UFUNCTION(BlueprintCallable)
	void CreateMySession(FString displayName, int32 playerCount);
	//세션 생성 완료 함수
	void OnCreateSessionComplete(FName sessionName, bool success);

	//세션 조회 
	//세션 조회할 때 사용하는 객체
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	//세션 조회 함수
	UFUNCTION(BlueprintCallable)
	void FindOtherSession();
	//세션 조회 완료 함수
	void OnFindSessionComplete(bool success);

	//세션 참여 
	//세션 참여 함수
	UFUNCTION(BlueprintCallable)
	void JoinOtherSession(int32 sessionIndex);
	//세션 참여 완료 함수
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);

	//=============이름 저장 부분================
public:
	void SetPlayerNickname(const FString& InName);
	FString GetPlayerNickname();

private:
	FString PlayerNickname;


	//=============플레이어 목록 관리 부분================
public:
	FOnPlayerListUpdated OnPlayerListUpdated;

	UFUNCTION(BlueprintCallable)
	void RequestPlayerListRefresh();

	void SetPlayerListManager(class AYiSanPlayerListManager* InManager);
	class AYiSanPlayerListManager* GetPlayerListManager() const { return PlayerListManager; }

private:
	UPROPERTY()
	class AYiSanPlayerListManager* PlayerListManager;

	void HandlePlayerListUpdated(const TArray<FString>& PlayerNames);
};
