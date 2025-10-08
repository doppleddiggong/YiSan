// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UVoiceComponent.generated.h"





/**
 * @brief Actor component that orchestrates voice capture lifecycle and observability.
 *
 * The component composes with existing character systems (stats, sight, hit-stop) so gameplay teams
 * can enrich voice context without duplicating plumbing before network integration arrives.
 * @ingroup Character
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Dopple), meta=(BlueprintSpawnableComponent))
class YISAN_API UVoiceSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	/** @brief Configure defaults and disable ticking for performance. */
	UVoiceSystem();

public:
	/**
	 * @brief Starts a voice capture session.
	 * @param ContextTag Context string that identifies the session (UI, quest, etc.).
	 */
	UFUNCTION(BlueprintCallable, Category="Voice|Lifecycle")
	void StartCapture(const FString& ContextTag);

	/** @brief Stops the active voice capture session. */
	UFUNCTION(BlueprintCallable, Category="Voice|Lifecycle")
	void StopCapture();

	/**
	 * @brief Submits a transcript coming from STT.
	 * @param TranscriptText Raw text from the recognizer.
	 * @param Confidence Recognizer confidence in range [0, 1].
	 */
	UFUNCTION(BlueprintCallable, Category="Voice|Processing")
	void SubmitTranscription(const FString& TranscriptText, float Confidence);

	/**
	 * @brief Delivers the final inference response (e.g. GPT output).
	 * @param ResponseText Formatted answer returned by the model.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice|Processing")
	void NotifyResponseReady(const FString& ResponseText);

	/** @brief Resets every cached voice state without firing delegates. */
	UFUNCTION(BlueprintCallable, Category="Voice|Lifecycle")
	void ResetVoiceState();

	/** @brief Returns whether capture is active. */
	UFUNCTION(BlueprintPure, Category="Voice|State")
	bool IsCapturing() const { return bIsCapturing; }

	/** @brief Returns the correlation id bound to the current session. */
	UFUNCTION(BlueprintPure, Category="Voice|State")
	FString GetActiveCorrelationId() const { return ActiveCorrelationId; }

public:
	/** @brief Broadcast when voice capture starts. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceCaptureStarted, const FString&, CorrelationId);
	/** @brief Delegate fired when voice capture starts. */
	UPROPERTY(BlueprintAssignable, Category="Voice|Delegates")
	FOnVoiceCaptureStarted OnVoiceCaptureStarted;

	/** @brief Broadcast when voice capture stops. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceCaptureStopped, const FString&, CorrelationId);
	/** @brief Delegate fired when voice capture stops. */
	UPROPERTY(BlueprintAssignable, Category="Voice|Delegates")
	FOnVoiceCaptureStopped OnVoiceCaptureStopped;

	/** @brief Broadcast when a new STT transcript is available. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVoiceTranscriptReceived, const FString&, CorrelationId, const FString&, TranscriptText, float, Confidence);
	/** @brief Delegate fired when a transcript is pushed in. */
	UPROPERTY(BlueprintAssignable, Category="Voice|Delegates")
	FOnVoiceTranscriptReceived OnVoiceTranscriptReceived;

	/** @brief Broadcast when a GPT/inference response is ready. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceResponseReceived, const FString&, CorrelationId, const FString&, ResponseText);
	/** @brief Delegate fired when an inference response is ready. */
	UPROPERTY(BlueprintAssignable, Category="Voice|Delegates")
	FOnVoiceResponseReceived OnVoiceResponseReceived;

protected:
	/** @brief Ensures the internal state is clean on BeginPlay. */
	virtual void BeginPlay() override;

private:
	/** @brief Writes attempt/outcome rows using the project's observability format. */
	void LogAttempt(const FString& Operation,
	                int32 Attempt,
	                int32 MaxAttempts,
	                double DurationMs,
	                const FString& Outcome,
	                const FString& Message,
	                const FString& ErrorType,
	                const FString& CorrelationId) const;

	/** @brief Generates a new correlation id using GUIDs. */
	FString GenerateCorrelationId() const;

	/**
	 * @brief Adds a transcript entry and respects the max history size.
	 * @param TranscriptText Text to append.
	 * @param Confidence Confidence associated with the text.
	 */
	void AppendToHistory(const FString& TranscriptText, float Confidence);

	/**
	 * @brief Tracks repeated failures and emits FailureDigest when thresholds are hit.
	 * @param NowSeconds Real time since world start.
	 * @param Operation Name of the failing operation.
	 */
	void TrackFailure(double NowSeconds, const FString& Operation);

private:
	/** @brief Max retry count before raising stronger signals. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Voice|Config", meta=(ClampMin="1", AllowPrivateAccess="true"))
	int32 MaxAttempts = 3;

	/** @brief Max amount of transcript entries to keep in memory. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Voice|Config", meta=(ClampMin="1", AllowPrivateAccess="true"))
	int32 MaxTranscriptHistory = 10;

	/** @brief True while capture is active. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Voice|State", meta=(AllowPrivateAccess="true"))
	bool bIsCapturing = false;

	/** @brief Correlation id shared with downstream systems. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Voice|State", meta=(AllowPrivateAccess="true"))
	FString ActiveCorrelationId;

	/** @brief Optional context tag tied to the session. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Voice|State", meta=(AllowPrivateAccess="true"))
	FString ActiveContextTag;

	/** @brief Rolling transcript history, formatted for quick dashboards. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Voice|State", meta=(AllowPrivateAccess="true"))
	TArray<FString> TranscriptHistory;

	/** @brief Retry counter used while a capture session is already running. */
	int32 RetryCounter = 0;

	/** @brief Consecutive failure counter across operations. */
	int32 ConsecutiveFailureCount = 0;

	/** @brief Failure timestamps inside the tracking window. */
	TArray<double> FailureTimestamps;

	/** @brief Real-time seconds when the last FailureDigest was emitted. */
	double LastFailureDigestSeconds = 0.0;
};
