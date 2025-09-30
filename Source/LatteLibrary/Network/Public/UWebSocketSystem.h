#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "ENetworkLogType.h"
#include "IWebSocket.h"
#include "UWebSocketSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketConnectionError, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWebSocketClosed, int32, StatusCode, const FString&, Reason, bool, bWasClean);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessageReceived, const FString&, Message);

UCLASS(Blueprintable, BlueprintType)
class LATTELIBRARY_API UWebSocketSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UWebSocketSystem);

    
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void Connect(const FString& Url, const FString& Protocol);

    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void Close();

    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void SendMessage(const FString& Message);

    UFUNCTION(BlueprintPure, Category = "WebSocket")
    bool IsConnected() const;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
    FOnWebSocketConnected OnConnected;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
    FOnWebSocketConnectionError OnConnectionError;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
    FOnWebSocketClosed OnClosed;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
    FOnWebSocketMessageReceived OnMessageReceived;

private:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    void OnConnected_Native();
    void OnConnectionError_Native(const FString& Error);
    void OnClosed_Native(int32 StatusCode, const FString& Reason, bool bWasClean);
    void OnMessageReceived_Native(const FString& Message);

    TSharedPtr<IWebSocket> WebSocket;


private:
    static void LogNetwork( const FString& Message );
};
