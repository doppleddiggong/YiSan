// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UYisanOnlineSystem.generated.h"

/**
 * @file UYisanOnlineSystem.h
 * @brief UYisanOnlineSystem 클래스를 선언합니다.
 */

DECLARE_DELEGATE_TwoParams(FFindComplete, int32, FString);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerListUpdated, const TArray<FString>& /* PlayerNames */);

/**
 * @brief 언리얼 온라인 서브시스템(Online Subsystem)을 기반으로 세션 생성, 검색, 참여 등 멀티플레이어 세션 관리를 담당하는 서브시스템입니다.
 * @details 플레이어 목록을 관리하고, 세션 관련 UI와 상호작용하며, 네트워크 연결의 핵심 로직을 처리합니다.
 */
UCLASS()
class YISAN_API UYisanOnlineSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UYisanOnlineSystem);

        /** @brief 서브시스템 델리게이트를 초기화하고 세션 인터페이스를 캐시합니다. */
        virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
public:
	FFindComplete OnFindComplete;

	//세션의 모든 처리 진행 객체
	IOnlineSessionPtr sessionInterface;

	//세션 생성 
	//세션 생성 함수
        /** @brief 다른 플레이어가 참가할 수 있는 새 세션을 생성합니다. */
        UFUNCTION(BlueprintCallable)
        void CreateMySession(FString displayName, int32 playerCount);
        //세션 생성 완료 함수
        void OnCreateSessionComplete(FName sessionName, bool success);

	//세션 조회 
	//세션 조회할 때 사용하는 객체
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	//세션 조회 함수
        /** @brief 온라인 서브시스템에서 다른 YiSan 세션을 검색합니다. */
        UFUNCTION(BlueprintCallable)
        void FindOtherSession();
        //세션 조회 완료 함수
        void OnFindSessionComplete(bool success);

	//세션 참여 
	//세션 참여 함수
        /** @brief 캐시된 검색 결과에서 세션 참가를 시도합니다. */
        UFUNCTION(BlueprintCallable)
        void JoinOtherSession(int32 sessionIndex);
        //세션 참여 완료 함수
        void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);

        // IP로 세션 참여 함수
        UFUNCTION(BlueprintCallable)
        void JoinSessionByIp(const FString& IpAddress);

	//=============이름 저장 부분================
public:
        /** @brief 세션 생성을 위해 로컬 플레이어 닉네임을 저장합니다. */
        void SetPlayerNickname(const FString& InName);
        /** @brief 캐시된 로컬 닉네임을 반환합니다. */
        FString GetPlayerNickname() { return PlayerNickname; }

private:
	FString PlayerNickname;


	//=============플레이어 목록 관리 부분================
public:
	FOnPlayerListUpdated OnPlayerListUpdated;

	UFUNCTION(BlueprintCallable)
        /** @brief 서버 또는 캐시에서 현재 플레이어 명단을 요청합니다. */
        void RequestRefreshPlayerList();

        /** @brief 게임 상태에서 전달된 플레이어 목록 업데이트를 처리합니다. */
        void HandlePlayerListUpdated(const TArray<FString>& PlayerNames);

        /** @brief 복제된 게임 상태 이벤트를 구독합니다. */
        void SetGameState(class AYisanGameState* InGameState);
        /** @brief 캐시된 게임 상태 참조가 있으면 반환합니다. */
        class AYisanGameState* GetGameState() const { return CachedGameState; }

private:
	UPROPERTY()
	TObjectPtr<class AYisanGameState> CachedGameState;
};