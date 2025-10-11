// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UHttpSystem.h
 * @brief HTTP 통신 시스템 선언
 */
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "NetworkData.h"
#include "Macro.h"
#include "UHttpSystem.generated.h"

UCLASS(Blueprintable, BlueprintType)
class YISAN_API UHttpSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UHttpSystem);

	// --- HTTP Request Methods ---

	/** Health Check: GET /health */
	UFUNCTION(BlueprintCallable, Category = "Http|Health")
	void RequestHealth(FResponseHealthDelegate OnComplete);

	/** Help Chat: POST /help_chat */
	UFUNCTION(BlueprintCallable, Category = "Http|Chat")
	void RequestHelpChat(const FRequestHelpChat& Request, FResponseHelpChatDelegate OnComplete);

	/** Speech-to-Text: POST /stt (Multipart file upload) */
	UFUNCTION(BlueprintCallable, Category = "Http|Voice")
	void RequestSTT(const FString& FilePath, FResponseSTTDelegate OnComplete);

	/** Text-to-Speech Simple: POST /fish/tts-simple */
	UFUNCTION(BlueprintCallable, Category = "Http|Voice")
	void RequestFishTTSSimple(const FRequestFishTTSSimple& Request, FResponseTTSDelegate OnComplete);

	/** Text-to-Speech: POST /tts */
	UFUNCTION(BlueprintCallable, Category = "Http|Voice")
	void RequestTTS(const FRequestTTS& Request, FResponseTTSDelegate OnComplete);

	/** Generic POST request with JSON body */
	UFUNCTION(BlueprintCallable, Category = "Http|Generic")
	void PostJSON(const FString& Endpoint, const FString& JsonBody, const FString& OnCompleteCallbackName);

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- HTTP Response Handlers ---
	void OnHealthResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FResponseHealthDelegate Callback);
	void OnHelpChatResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FResponseHelpChatDelegate Callback);
	void OnSTTResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FResponseSTTDelegate Callback);
	void OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FResponseTTSDelegate Callback);

	// --- Helper Methods ---
	static void LogHttp(const FString& Message);
	static FString ConvertStructToJsonString(const void* StructPtr, UScriptStruct* StructType);
	static TArray<uint8> LoadFileToByteArray(const FString& FilePath);
};
