// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UVoiceSystem.cpp
 * @brief Implementation of UVoiceSystem.
 */

#include "UVoiceComponent.h"

#include "GameLogging.h"
#include "HAL/PlatformTime.h"
#include "Misc/Guid.h"

namespace VoiceSystemInternal
{
	static constexpr double FailureWindowSeconds = 300.0; // 5 minutes
	static constexpr double FailureDigestCooldownSeconds = 60.0;
}

UVoiceSystem::UVoiceSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UVoiceSystem::BeginPlay()
{
	Super::BeginPlay();

	ResetVoiceState();

	LogAttempt(TEXT("Voice.BeginPlay"), 1, MaxAttempts, 0.0, TEXT("Success"), TEXT("VoiceSystem ready."), TEXT("None"), GenerateCorrelationId());
}

void UVoiceSystem::StartCapture(const FString& ContextTag)
{
	const double BeginSeconds = FPlatformTime::Seconds();
	const double NowSeconds = GetWorld() ? GetWorld()->GetRealTimeSeconds() : BeginSeconds;

	if (bIsCapturing)
	{
		++RetryCounter;
		++ConsecutiveFailureCount;
		TrackFailure(NowSeconds, TEXT("Voice.StartCapture"));

		const double DurationMs = (FPlatformTime::Seconds() - BeginSeconds) * 1000.0;
		const FString Message = FString::Printf(TEXT("Capture already active. Requested Context=%s, ActiveContext=%s"), *ContextTag, *ActiveContextTag);
		LogAttempt(TEXT("Voice.StartCapture"), RetryCounter, MaxAttempts, DurationMs, TEXT("Retry"), Message, TEXT("AlreadyActive"), ActiveCorrelationId);
		return;
	}

	bIsCapturing = true;
	RetryCounter = 0;
	ConsecutiveFailureCount = 0;
	FailureTimestamps.Reset();

	ActiveCorrelationId = GenerateCorrelationId();
	ActiveContextTag = ContextTag;

	OnVoiceCaptureStarted.Broadcast(ActiveCorrelationId);

	const double DurationMs = (FPlatformTime::Seconds() - BeginSeconds) * 1000.0;
	const FString Message = FString::Printf(TEXT("Voice capture started. Context=%s"), *ContextTag);
	LogAttempt(TEXT("Voice.StartCapture"), 1, MaxAttempts, DurationMs, TEXT("Success"), Message, TEXT("None"), ActiveCorrelationId);
}

void UVoiceSystem::StopCapture()
{
	const double BeginSeconds = FPlatformTime::Seconds();
	const double NowSeconds = GetWorld() ? GetWorld()->GetRealTimeSeconds() : BeginSeconds;

	if (!bIsCapturing)
	{
		++ConsecutiveFailureCount;
		TrackFailure(NowSeconds, TEXT("Voice.StopCapture"));

		const double DurationMs = (FPlatformTime::Seconds() - BeginSeconds) * 1000.0;
		LogAttempt(TEXT("Voice.StopCapture"), ConsecutiveFailureCount, MaxAttempts, DurationMs, TEXT("Fail"), TEXT("StopCapture called without active session."), TEXT("InvalidState"), ActiveCorrelationId);
		return;
	}

	const FString CorrelationId = ActiveCorrelationId;

	bIsCapturing = false;
	ActiveCorrelationId.Reset();
	ActiveContextTag.Reset();
	RetryCounter = 0;
	ConsecutiveFailureCount = 0;
	FailureTimestamps.Reset();

	OnVoiceCaptureStopped.Broadcast(CorrelationId);

	const double DurationMs = (FPlatformTime::Seconds() - BeginSeconds) * 1000.0;
	LogAttempt(TEXT("Voice.StopCapture"), 1, MaxAttempts, DurationMs, TEXT("Success"), TEXT("Voice capture stopped."), TEXT("None"), CorrelationId);
}

void UVoiceSystem::SubmitTranscription(const FString& TranscriptText, float Confidence)
{
	const double BeginSeconds = FPlatformTime::Seconds();
	const double NowSeconds = GetWorld() ? GetWorld()->GetRealTimeSeconds() : BeginSeconds;
	const FString CorrelationId = ActiveCorrelationId.IsEmpty() ? GenerateCorrelationId() : ActiveCorrelationId;

	if (TranscriptText.TrimStartAndEnd().IsEmpty())
	{
		++ConsecutiveFailureCount;
		TrackFailure(NowSeconds, TEXT("Voice.SubmitTranscription"));

		const double DurationMs = (FPlatformTime::Seconds() - BeginSeconds) * 1000.0;
		LogAttempt(TEXT("Voice.SubmitTranscription"), ConsecutiveFailureCount, MaxAttempts, DurationMs, TEXT("Fail"), TEXT("Transcript text is empty."), TEXT("InvalidPayload"), CorrelationId);
		return;
	}

	AppendToHistory(TranscriptText, Confidence);
	OnVoiceTranscriptReceived.Broadcast(CorrelationId, TranscriptText, FMath::Clamp(Confidence, 0.0f, 1.0f));

	ConsecutiveFailureCount = 0;
	FailureTimestamps.Reset();

	const double DurationMs = (FPlatformTime::Seconds() - BeginSeconds) * 1000.0;
	const FString Message = FString::Printf(TEXT("Transcript accepted. Length=%d"), TranscriptText.Len());
	LogAttempt(TEXT("Voice.SubmitTranscription"), 1, MaxAttempts, DurationMs, TEXT("Success"), Message, TEXT("None"), CorrelationId);
}

void UVoiceSystem::NotifyResponseReady(const FString& ResponseText)
{
	const double BeginSeconds = FPlatformTime::Seconds();
	const double NowSeconds = GetWorld() ? GetWorld()->GetRealTimeSeconds() : BeginSeconds;
	const FString CorrelationId = ActiveCorrelationId.IsEmpty() ? GenerateCorrelationId() : ActiveCorrelationId;

	if (ResponseText.TrimStartAndEnd().IsEmpty())
	{
		++ConsecutiveFailureCount;
		TrackFailure(NowSeconds, TEXT("Voice.NotifyResponseReady"));

		const double DurationMs = (FPlatformTime::Seconds() - BeginSeconds) * 1000.0;
		LogAttempt(TEXT("Voice.NotifyResponseReady"), ConsecutiveFailureCount, MaxAttempts, DurationMs, TEXT("Fail"), TEXT("Response text is empty."), TEXT("InvalidPayload"), CorrelationId);
		return;
	}

	OnVoiceResponseReceived.Broadcast(CorrelationId, ResponseText);

	ConsecutiveFailureCount = 0;
	FailureTimestamps.Reset();

	const double DurationMs = (FPlatformTime::Seconds() - BeginSeconds) * 1000.0;
	const FString Message = FString::Printf(TEXT("Response delivered. Length=%d"), ResponseText.Len());
	LogAttempt(TEXT("Voice.NotifyResponseReady"), 1, MaxAttempts, DurationMs, TEXT("Success"), Message, TEXT("None"), CorrelationId);
}

void UVoiceSystem::ResetVoiceState()
{
	bIsCapturing = false;
	ActiveCorrelationId.Reset();
	ActiveContextTag.Reset();
	TranscriptHistory.Reset();
	RetryCounter = 0;
	ConsecutiveFailureCount = 0;
	FailureTimestamps.Reset();
	LastFailureDigestSeconds = 0.0;
}

void UVoiceSystem::LogAttempt(const FString& Operation,
                              int32 Attempt,
                              int32 MaxAttemptsIn,
                              double DurationMs,
                              const FString& Outcome,
                              const FString& Message,
                              const FString& ErrorType,
                              const FString& CorrelationId) const
{
	const FString Context = TEXT("Context");
	const FString CombinedMessage = Message.IsEmpty() ? Context : FString::Printf(TEXT("%s | %s"), *Message, *Context);

	const FString SanitizedMessage = CombinedMessage.Replace(TEXT("\""), TEXT("\\\""));
	const FString SanitizedOutcome = Outcome.Replace(TEXT("\""), TEXT("\\\""));
	const FString SanitizedError = ErrorType.Replace(TEXT("\""), TEXT("\\\""));
	const FString SanitizedCorrelation = CorrelationId.IsEmpty() ? TEXT("N/A") : CorrelationId;

	const FString Formatted = FString::Printf(
		TEXT("{ \"Operation\": \"%s\", \"Attempt\": %d, \"MaxAttempts\": %d, \"DurationMs\": %.2f, \"Outcome\": \"%s\", \"ErrorType\": \"%s\", \"CorrelationId\": \"%s\", \"Message\": \"%s\" }"),
		*Operation,
		Attempt,
		MaxAttemptsIn,
		DurationMs,
		*SanitizedOutcome,
		*SanitizedError,
		*SanitizedCorrelation,
		*SanitizedMessage);

	PRINTLOG(TEXT("[VoiceSystem] %s"), *Formatted);
}

FString UVoiceSystem::GenerateCorrelationId() const
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
}

void UVoiceSystem::AppendToHistory(const FString& TranscriptText, float Confidence)
{
	const float ClampedConfidence = FMath::Clamp(Confidence, 0.0f, 1.0f);
	const FString HistoryLabel = ActiveContextTag.IsEmpty() ? TEXT("Default") : ActiveContextTag;
	const FString CorrelationLabel = ActiveCorrelationId.IsEmpty() ? TEXT("Transient") : ActiveCorrelationId;
	const FString Entry = FString::Printf(TEXT("[%s][%s](%.2f) %s"), *HistoryLabel, *CorrelationLabel, ClampedConfidence, *TranscriptText);

	TranscriptHistory.Add(Entry);

	if (TranscriptHistory.Num() > MaxTranscriptHistory)
	{
		const int32 Overflow = TranscriptHistory.Num() - MaxTranscriptHistory;
		TranscriptHistory.RemoveAt(0, Overflow,  EAllowShrinking::No);
	}
}

void UVoiceSystem::TrackFailure(double NowSeconds, const FString& Operation)
{
	FailureTimestamps.RemoveAll([NowSeconds](double Timestamp)
	{
		return (NowSeconds - Timestamp) > VoiceSystemInternal::FailureWindowSeconds;
	});

	FailureTimestamps.Add(NowSeconds);

	if ((ConsecutiveFailureCount >= 5 || FailureTimestamps.Num() >= 5) &&
	    (LastFailureDigestSeconds <= 0.0 || (NowSeconds - LastFailureDigestSeconds) > VoiceSystemInternal::FailureDigestCooldownSeconds))
	{
		LastFailureDigestSeconds = NowSeconds;

		const FString DigestMessage = FString::Printf(TEXT("FailureDigest | Operation=%s | Failures(5m)=%d | Consecutive=%d"), *Operation, FailureTimestamps.Num(), ConsecutiveFailureCount);
		LogAttempt(Operation + TEXT(".FailureDigest"), ConsecutiveFailureCount, MaxAttempts, 0.0, TEXT("FailureDigest"), DigestMessage, TEXT("FailureDigest"), ActiveCorrelationId);
	}
}