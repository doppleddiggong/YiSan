// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UYisanOnlineSystem.h"

#include "TimerManager.h"
#include "ContentStreaming.h"

#include "AYisanGameState.h"
#include "GameLogging.h"
#include "Widgets/SWidget.h"
#include "GameFramework/PlayerController.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "UDialogManager.h"
#include "ULoadingCircleManager.h"
#include "Online/OnlineSessionNames.h"

/**
 * @file UYisanOnlineSystem.cpp
 * @brief UYisanOnlineSystem의 동작을 구현합니다.
 */

// ==================== Network 관리 ====================
void UYisanOnlineSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if( auto subsys = Online::GetSubsystem(GetWorld()) )
    {
        //서브시스템의 인터페이스를 가져오자
        sessionInterface = subsys->GetSessionInterface();
        //세션생성 성공시 호출되는 함수 등록
        sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UYisanOnlineSystem::OnCreateSessionComplete);
        //세션조회 성공시 호출되는 함수 등록
        sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UYisanOnlineSystem::OnFindSessionComplete);
        //세션참여 성공시 호출되는 함수 등록
        sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UYisanOnlineSystem::OnJoinSessionComplete);
    }
}

/** @brief 플레이어 목록 업데이트를 위해 시스템을 복제된 게임 상태에 연결합니다. */
void UYisanOnlineSystem::SetGameState(AYisanGameState* InGameState)
{
    if (CachedGameState)
    {
        CachedGameState->OnPlayerListUpdated.RemoveAll(this);
    }
    CachedGameState = InGameState;
    if (CachedGameState)
    {
        CachedGameState->OnPlayerListUpdated.AddUObject(this, &UYisanOnlineSystem::HandlePlayerListUpdated);
    }
}

/** @brief 게임 상태가 준비되지 않았을 경우 재시도하며 최신 플레이어 목록을 요청합니다. */
void UYisanOnlineSystem::RequestRefreshPlayerList()
{
    if (CachedGameState)
    {
        TArray<FString> PlayerList = CachedGameState->GetPlayerList();
        if (PlayerList.Num() > 0)
            OnPlayerListUpdated.Broadcast(PlayerList);
        else
            CachedGameState->RequestRefreshPlayerList();
        return;
    }

    PRINTLOG(TEXT("RequestPlayerListRefresh Failed!"));

    if (UWorld* World = GetWorld())
    {
        FTimerHandle RetryTimerHandle;
        World->GetTimerManager().SetTimer(RetryTimerHandle, [this]()
        {
            if (CachedGameState)
            {
                PRINTLOG(TEXT("RequestRefreshPlayerList - Retry Succeeded"));
                auto PlayerList = CachedGameState->GetPlayerList();
                if (PlayerList.Num() > 0)
                    OnPlayerListUpdated.Broadcast(PlayerList);
                else
                    CachedGameState->RequestRefreshPlayerList();
            }
            else
            {
                PRINTLOG(TEXT("RequestPlayerListRefresh - Still Failed"));
            }
        }, 0.5f, false);
    }
}

/** @brief 플레이어 목록 업데이트를 로컬 리스너에 전달합니다. */
void UYisanOnlineSystem::HandlePlayerListUpdated(const TArray<FString>& PlayerNames)
{
    OnPlayerListUpdated.Broadcast(PlayerNames);
}

/** @brief 제공된 표시 이름과 수용 인원으로 로비 세션을 생성합니다. */
void UYisanOnlineSystem::CreateMySession(FString displayName, int32 playerCount)
{
    if (!sessionInterface.IsValid())
    {
        PRINTLOG( TEXT("SessionInterface is invalid!"));
        return;
    }
    
    FOnlineSessionSettings sessionSettings;
    FName subsysName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
    sessionSettings.bIsLANMatch = subsysName.IsEqual(FName(TEXT("NULL")));
    sessionSettings.bUseLobbiesIfAvailable = true;
    sessionSettings.bUsesPresence = true;
    sessionSettings.bShouldAdvertise = true;
    sessionSettings.NumPublicConnections = playerCount;
    sessionSettings.Set(FName("DP_NAME"), displayName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
    FString netIDString = netID->ToString();
    PRINTLOG( TEXT("서브시스템 : %s"), *subsysName.ToString());

    sessionInterface->CreateSession(0, FName(displayName), sessionSettings);
}

/** @brief 세션 생성 요청 완료를 처리합니다. */
void UYisanOnlineSystem::OnCreateSessionComplete(FName sessionName, bool success)
{
    PRINTLOG( TEXT("OnCreateSessionComplete(%s, %d)"), *sessionName.ToString(), success );

    if (success)
    {
        if (auto DM = UDialogManager::Get(GetWorld()))
            DM->ShowToast(FString::Printf( TEXT("세션 : %s 성공"), *sessionName.ToString()));

        // Absolute travel for new session (Lobby -> StartLevel)
        GetWorld()->ServerTravel(TEXT("/Game/CustomContents/Maps/StartLevel?listen"), true);
    }
    else
    {
        PRINTLOG( TEXT("OnCreateSessionComplete FAILED!!!"));
    }
}

/** @brief 사용 가능한 다른 세션 검색을 시작합니다. */
void UYisanOnlineSystem::FindOtherSession()
{
    PRINTLOG( TEXT("FindOtherSession()"));

    if (auto DM = UDialogManager::Get(GetWorld()))
        DM->ShowToast(TEXT("조회 시작"));        
    
    //sessionSearch 만들자
    sessionSearch = MakeShared<FOnlineSessionSearch>();
    FName subsysName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
    sessionSearch->bIsLanQuery = subsysName.IsEqual(FName(TEXT("NULL")));
    sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    sessionSearch->MaxSearchResults = 100;
    sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

/** @brief 세션 검색 결과를 처리합니다. */
void UYisanOnlineSystem::OnFindSessionComplete(bool success)
{
    PRINTLOG( TEXT("OnFindSessionComplete(%d)"), success );

    if (UDialogManager* DM = UDialogManager::Get(GetWorld()))
        DM->ShowToast(TEXT("조회 완료"));        
    
    if (success)
    {
        if (sessionSearch.IsValid()) 
        { // Check if sessionSearch is valid
            auto results = sessionSearch->SearchResults;
	        for (int32 i = 0; i < results.Num(); i++)
	        {
	            //세션이름담을변수
	            FString displayName;
	            results[i].Session.SessionSettings.Get(FName(TEXT("DP_NAME")), displayName);

	            PRINTLOG( TEXT("세션 : %i, 이름 : %s"), i, *displayName);
	            OnFindComplete.ExecuteIfBound(i, displayName);
	        }
        }
    }

    OnFindComplete.ExecuteIfBound(-1, FString());
}

/** @brief 검색 결과 인덱스로 다른 세션 참가를 시도합니다. */
void UYisanOnlineSystem::JoinOtherSession(int32 sessionIndex)
{
    auto results = sessionSearch->SearchResults;
    if (results.Num() <= 0)
    {
        PRINTLOG( TEXT("No sessions found! Cannot join."));
        return;
    }
    results[sessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable = true;
    results[sessionIndex].Session.SessionSettings.bUsesPresence = true;

    FString displayName;
    results[sessionIndex].Session.SessionSettings.Get(FName(TEXT("DP_NAME")), displayName);

    ULoadingCircleManager::Get(GetWorld())->Show();
    
    sessionInterface->JoinSession(0, FName(displayName), results[sessionIndex]);
}

/** @brief 세션 참가 작업에 응답하고 이동 또는 오류를 처리합니다. */
void UYisanOnlineSystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
    ULoadingCircleManager::Get(GetWorld())->Hide();

    if (result == EOnJoinSessionCompleteResult::Success)
    {
        FString url;
        sessionInterface->GetResolvedConnectString(sessionName, url);
        PRINTLOG( TEXT("URL : %s"), *url);

        if (auto Ctrl = GetWorld()->GetFirstPlayerController())
        {
            Ctrl->ClientTravel(url, TRAVEL_Absolute);
        }
    }
    else
    {
        PRINTLOG( TEXT("OnJoinSessionComplete(%d)"), result);
    }
}

void UYisanOnlineSystem::JoinSessionByIp(const FString& IpAddress)
{
    if (auto Ctrl = GetWorld()->GetFirstPlayerController())
    {
        Ctrl->ClientTravel(IpAddress, TRAVEL_Absolute);
    }
}

void UYisanOnlineSystem::SetPlayerNickname(const FString& InName)
{
    PRINTLOG(TEXT("SetPlayerNickname(%s)"), *PlayerNickname);
    PlayerNickname = InName;
}