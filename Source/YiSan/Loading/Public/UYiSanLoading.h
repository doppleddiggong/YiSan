// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UYiSanLoading.generated.h"

UCLASS()
class YISAN_API UYiSanLoading : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UYiSanLoading);

    void InitSystem(const FString& InURL, bool bAbsolute);

private:
    enum class ELoadingSequenceStage : uint8
    {
        WorldPartition,
        Texture,
        LevelInstances,
        Completed
    };

    void ResetLoadingState();
    void PostLoadMapWithWorld(UWorld* World);
    void CompleteProcess(const UWorld* World);
    void Poll_StreamingAndInstancesReady();
    void Loading_Textures(const UWorld* World);
    void Loading_LevelInstance(UWorld* World);
    void UpdateLoadingProgress();
    float CalculateTotalProgress() const;
    void AdvanceToStage(ELoadingSequenceStage NextStage);

    void MarkStageStart(ELoadingSequenceStage Stage, double TimeSeconds);
    void MarkStageComplete(ELoadingSequenceStage Stage, double TimeSeconds);
    void LogStageSummary(const UWorld* World) const;
    const TCHAR* GetStageLabel(ELoadingSequenceStage Stage) const;

    void BroadcastLoadingScreenShow() const;
    void BroadcastLoadingScreenHide() const;
    void BroadcastLoadingProgress(float Progress) const;

private:
    struct FStageTiming
    {
        double StartTime = 0.0;
        double EndTime = 0.0;
        bool bStarted = false;
        bool bCompleted = false;

        double GetDuration() const
        {
            return (bStarted && bCompleted) ? EndTime - StartTime : 0.0;
        }
    };

    static constexpr int32 StageCount = static_cast<int32>(ELoadingSequenceStage::Completed) + 1;

    static constexpr double TextureStreamingTimeoutSeconds = 60.0;
    static constexpr double TextureProgressLogIntervalSeconds = 5.0;
    static constexpr float WorldPartitionWeight = 0.15f;
    static constexpr float TextureWeight = 0.70f;
    static constexpr float LevelInstanceWeight = 0.15f;

    double ResourceCheckStartTime = 0.0;
    bool bInitialTextureStreamingComplete = false;

    float TotalProgress = 0.0f;

    bool bWorldPartitionReady = false;
    bool bTextureReady = false;
    bool bLevelInstancesReady = false;
    bool bCompletionAnnounced = false;
    float StreamingPercentage = 0.0f;
    float LevelInstanceProgress = 0.0f;

    bool bCapturedInitialTextureRequests = false;
    int32 InitialTextureRequestCount = 0;
    double LastTextureProgressLogTime = 0.0;
    float LastReportedTextureProgress = -1.0f;

    FTimerHandle TimeHandlePool;
    ELoadingSequenceStage CurrentStage = ELoadingSequenceStage::WorldPartition;

    int32 LastReportedPercent = -10;

    FStageTiming StageTimings[StageCount] = {};

    UPROPERTY()
    TObjectPtr<class UDialogManager> DM;
};
