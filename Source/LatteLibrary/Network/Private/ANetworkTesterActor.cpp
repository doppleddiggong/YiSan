// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ANetworkTesterActor.h"
#include "UHttpNetworkSystem.h"
#include "Shared/GameLogging.h"
#include "NetworkData.h"
#include "UGameDataManager.h"
#include "UGameFunctionLibrary.h"
#include "Engine/Engine.h"

ANetworkTesterActor::ANetworkTesterActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ANetworkTesterActor::RequestHealth()
{
    if ( auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()) )
    {
        ReqNetwork->RequestHealth( FResponseHealthDelegate::CreateUObject( this, &ANetworkTesterActor::OnResponseHealth));
    }
    else
    {
        PRINTLOG( TEXT("UNetworkSystem not found!"));
    }
}

void ANetworkTesterActor::OnResponseHealth(FResponseHealth& ResponseData, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        ResponseData.PrintData();
    }
    else
    {
        PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
    }
}

void ANetworkTesterActor::RequestHelpChat( const FString InQuestion )
{
    if ( auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()) )
    {
        FRequestHelpChat RequestData;
        RequestData.question = InQuestion;
        
        ReqNetwork->RequestHelpChat(
            RequestData,
            FResponseHelpChatDelegate::CreateUObject( this, &ANetworkTesterActor::OnResponseHelpChat) );
    }
    else
    {
        PRINTLOG( TEXT("UNetworkSystem not found!"));
    }
}

void ANetworkTesterActor::OnResponseHelpChat(FResponseHelpChat& ResponseData, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        ResponseData.PrintData();
    }
    else
    {
        PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
    }
}
