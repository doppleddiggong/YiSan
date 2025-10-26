// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UNetworkGameInstanceSubsystem.h"

#include "TimerManager.h"
#include "ContentStreaming.h"

// #include "UObject/WeakObjectPtr.h"
// #include "Templates/SharedPointer.h" 
#include "Widgets/SWidget.h"
#include "GameFramework/PlayerController.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "UBroadcastManager.h"
#include "UDialogManager.h"
#include "GameFramework/GameSession.h"
#include "Online/OnlineSessionNames.h"

// ==================== Network 관리 ====================
void UNetworkGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if( auto subsys = Online::GetSubsystem(GetWorld()) )
    {
        //서브시스템의 인터페이스를 가져오자
        sessionInterface = subsys->GetSessionInterface();
        //세션생성 성공시 호출되는 함수 등록
        sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetworkGameInstanceSubsystem::OnCreateSessionComplete);
        //세션조회 성공시 호출되는 함수 등록
        sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetworkGameInstanceSubsystem::OnFindSessionComplete);
        //세션참여 성공시 호출되는 함수 등록
        sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UNetworkGameInstanceSubsystem::OnJoinSessionComplete);
    }
}

void UNetworkGameInstanceSubsystem::CreateMySession(FString displayName, int32 playerCount)
{
    if (!sessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid!"));
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
    UE_LOG(LogTemp, Warning, TEXT("서브시스템 : %s"), *subsysName.ToString());
    /*UE_LOG(LogTemp, Warning, TEXT("netID : %s"), *netIDString);
    sessionInterface->CreateSession(*netID, FName(displayName), sessionSettings);*/
    sessionInterface->CreateSession(0, FName(displayName), sessionSettings);
}

void UNetworkGameInstanceSubsystem::OnCreateSessionComplete(FName sessionName, bool success)
{
    if (success)
    {
        UE_LOG(LogTemp, Warning, TEXT("세션 : %s 성공"), *sessionName.ToString());
        // Seamless Travel을 사용하여 클라이언트 연결 유지
        GetWorld()->ServerTravel(TEXT("/Game/CustomContents/Maps/StartLevel?listen"), true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("세션 : %s 실패"), *sessionName.ToString());
    }
}

void UNetworkGameInstanceSubsystem::FindOtherSession()
{
    UE_LOG(LogTemp, Warning, TEXT("세션 조회 시작"));


    
    
    //sessionSearch 만들자
    sessionSearch = MakeShared<FOnlineSessionSearch>();
    FName subsysName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
    sessionSearch->bIsLanQuery = subsysName.IsEqual(FName(TEXT("NULL")));
    sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    sessionSearch->MaxSearchResults = 100;
    sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UNetworkGameInstanceSubsystem::OnFindSessionComplete(bool success)
{
    UE_LOG(LogTemp, Warning, TEXT("세션 조회 끝"));
    if (success)
    {
        auto results = sessionSearch->SearchResults;
        for (int32 i = 0; i < results.Num(); i++)
        {
            //세션이름담을변수
            FString displayName;
            results[i].Session.SessionSettings.Get(FName(TEXT("DP_NAME")), displayName);
            UE_LOG(LogTemp, Warning, TEXT("세션 : %i, 이름 : %s"), i, *displayName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("세션 조회 실패"));
    }
}

void UNetworkGameInstanceSubsystem::JoinOtherSession(int32 sessionIndex)
{
    auto results = sessionSearch->SearchResults;
    if (results.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No sessions found! Cannot join."));
        return;
    }
    results[sessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable = true;
    results[sessionIndex].Session.SessionSettings.bUsesPresence = true;

    FString displayName;
    results[sessionIndex].Session.SessionSettings.Get(FName(TEXT("DP_NAME")), displayName);

    sessionInterface->JoinSession(0, FName(displayName), results[sessionIndex]);
}

void UNetworkGameInstanceSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
    if (result == EOnJoinSessionCompleteResult::Success)
    {
        FString url;
        sessionInterface->GetResolvedConnectString(sessionName, url);
        UE_LOG(LogTemp, Warning, TEXT("URL : %s"), *url)
        //서버가있는 맵으로 이동 (최초1회)
        APlayerController* pc = GetWorld()->GetFirstPlayerController();
        pc->ClientTravel(url, TRAVEL_Absolute);
    }
}


