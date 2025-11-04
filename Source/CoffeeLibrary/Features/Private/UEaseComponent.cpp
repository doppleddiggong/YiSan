// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UEaseComponent.cpp
 * @brief UEaseComponent의 동작을 구현합니다.
 */
#include "UEaseComponent.h"

/** @brief Tick 활성화를 설정한다. */
UEaseComponent::UEaseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

/** @brief 컴포넌트 Tick에서 모든 트랙을 업데이트한다. */
void UEaseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	this->UpdateTrack(DeltaTime);
}

/** @brief 등록된 모든 트랙에 대해 업데이트를 수행한다. */
void UEaseComponent::UpdateTrack(float DeltaTime)
{
	for (auto& Elem : FloatTracks)
	{
		Elem.Value.Update(DeltaTime);
	}

	for (auto& Elem : VectorTracks)
	{
		Elem.Value.Update(DeltaTime);
	}

	for (auto& Elem : RotatorTracks)
	{
		Elem.Value.Update(DeltaTime);
	}
}

/** @brief 주어진 이름의 트랙 진행도를 반환한다. */
float UEaseComponent::GetTrackAlpha(FName TrackName) const
{
	if (auto* ElemPtr = FloatTracks.Find(TrackName))
	{
		return ElemPtr->GetAlpha();
	}

	if (auto* ElemPtr = VectorTracks.Find(TrackName))
	{
		return ElemPtr->GetAlpha();
	}

	if (auto* ElemPtr = RotatorTracks.Find(TrackName))
	{
		return ElemPtr->GetAlpha();
	}

	return 0.f;
}

/** @brief float 트랙의 현재 값을 조회한다. */
float UEaseComponent::GetEaseFloatTrack(FName TrackName)
{
	return FloatTracks.Contains(TrackName) ? FloatTracks[TrackName].Current : 0.f;
}

/** @brief float 트랙을 초기화하거나 업데이트한다. */
void UEaseComponent::SetEaseFloatTrack(FName TrackName, EEaseType EaseType, float Start, float Target, float Duration)
{
	FEaseFloatTrack& Track = FloatTracks.FindOrAdd(TrackName);
	Track.EaseType = EaseType;
	Track.Start = Start;
	Track.Target = Target;
	Track.Duration = Duration;
	Track.ElapsedTime = 0.f;
}

/** @brief vector 트랙의 현재 값을 조회한다. */
FVector UEaseComponent::GetEaseVectorTrack(FName TrackName)
{
	return VectorTracks.Contains(TrackName) ? VectorTracks[TrackName].Current : FVector::ZeroVector;
}

/** @brief vector 트랙을 초기화하거나 업데이트한다. */
void UEaseComponent::SetEaseVectorTrack(FName TrackName, EEaseType EaseType, FVector Start, FVector Target, float Duration)
{
	FEaseVectorTrack& Track = VectorTracks.FindOrAdd(TrackName);
	Track.EaseType = EaseType;
	Track.Start = Start;
	Track.Target = Target;
	Track.Duration = Duration;
	Track.ElapsedTime = 0.f;
}

/** @brief rotator 트랙의 현재 값을 조회한다. */
FRotator UEaseComponent::GetEaseRotatorTrack(FName TrackName)
{
	return RotatorTracks.Contains(TrackName) ? RotatorTracks[TrackName].Current : FRotator::ZeroRotator;
}

/** @brief rotator 트랙을 초기화하거나 업데이트한다. */
void UEaseComponent::SetEaseRotatorTrack(FName TrackName, EEaseType EaseType, FRotator Start, FRotator Target, float Duration)
{
	FEaseRotatorTrack& Track = RotatorTracks.FindOrAdd(TrackName);
	Track.EaseType = EaseType;
	Track.Start = Start;
	Track.Target = Target;
	Track.Duration = Duration;
	Track.ElapsedTime = 0.f;
}