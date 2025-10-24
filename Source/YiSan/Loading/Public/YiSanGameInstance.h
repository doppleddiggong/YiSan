// Copyright (c) 2025 Doppleddiggong. 모든 권리 보유함.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "YiSanGameInstance.generated.h"

UCLASS()
class YISAN_API UYiSanGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	
#pragma region ONLINE_SUB_SYSTEM
public:
	void InitSubsystem();

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
	
#pragma endregion ONLINE_SUB_SYSTEM
	
};