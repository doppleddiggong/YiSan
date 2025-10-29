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

	void InitSystem(const FString& InURL, const bool bAbsolute);
	
private:
	// 맵 로드 완료후 호출 콜백.
	void PostLoadMapWithWorld(UWorld* World);
	
	// Step 3: 모든 스트리밍(WP, 텍스처, 레벨 인스턴스)이 완료된 후 호출됨.
	void CompleteProcess(const UWorld* World);

	// 리소스 체크 함수임
	// 스트리밍 및 레벨 인스턴스 준비 상태를 주기적으로 폴링함.
	void Poll_StreamingAndInstancesReady();

	// 로딩_텍스쳐
	void Loading_Textures(const UWorld* World);
	
	// 로딩_레벨 인스턴스
	void Loading_LevelInstance(UWorld* World);

	// 로딩 UI의 진행률과 상태 텍스트를 업데이트함 (현재는 로그만 출력).
	void UpdateLoadingProgress();

	void BroadcastLoadingScreenShow() const;
	void BroadcastLoadingScreenHide() const;
	void BroadcastLoadingProgress(float Progress) const;

private:
	// 타임아웃 체크를 위한 시작 시간
	double ResourceCheckStartTime = 0.0;
	
	// 텍스쳐 스트리밍 추적 변수
	double TextureStreamingStartTime = 0.0;
	bool bInitialTextureStreamingComplete = false;

	// 전체 진행율
	float TotalProgress;

	bool bWorldPartitionReady = false;
	bool bTextureReady = false;
	bool bLevelInstancesReady = false;
	float StreamingPercentage = 0.0f;
	float LevelInstanceProgress = 0.0f;

	// 스트리밍 완료 폴링 타이머 핸들임.
	FTimerHandle TimeHandlePool;

	int32 LastReportedPercent = -10;
	
	UPROPERTY()
	TObjectPtr<class UDialogManager> DM;
};
