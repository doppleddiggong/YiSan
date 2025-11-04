/**
 * @file UParabolaComponent.cpp
 * @brief UParabolaComponent의 동작을 구현합니다.
 */
#include "UParabolaComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

/** @brief Tick 활성화를 설정한다. */
UParabolaComponent::UParabolaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

/** @brief 초기화 시 모든 트랙의 시간을 리셋한다. */
void UParabolaComponent::BeginPlay()
{
	Super::BeginPlay();

	for (auto& P : BallisticTracks)
		P.Value.ResetTime();
	for (auto& P : GeometricTracks)
		P.Value.ResetTime();
}

/** @brief Tick마다 트랙을 갱신한다. */
void UParabolaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateTracks(DeltaTime);
}

/** @brief 모든 포물선 트랙의 시간을 진행시킨다. */
void UParabolaComponent::UpdateTracks(float DeltaTime)
{
	for (auto& P : BallisticTracks)
		P.Value.Advance(DeltaTime);
	for (auto& P : GeometricTracks)
		P.Value.Advance(DeltaTime);
}



/** @brief 지정된 트랙의 진행 방향을 기반으로 회전을 계산한다. */
FRotator UParabolaComponent::GetParabolaFacing(FName TrackName, bool bYawOnly, EForwardAxis ForwardAxis) const
{
	if (const FParabolaBallisticTrack* Ballistic = BallisticTracks.Find(TrackName))
	{
		const float CurAlpha  = FMath::Clamp(Ballistic->GetAlpha(), 0.f, 1.f);
		const float NextAlpha = FMath::Clamp(CurAlpha + 0.08f, 0.f, 1.f);

		if (FMath::IsNearlyEqual(CurAlpha, NextAlpha))
			return GetOwner()->GetActorRotation();

		const FVector CurLocation  = Ballistic->EvaluateAtAlpha(GetOwner(), CurAlpha);
		const FVector NextLocation = Ballistic->EvaluateAtAlpha(GetOwner(), NextAlpha);

		return MakeFacingFromDir(NextLocation - CurLocation, bYawOnly, ForwardAxis);
	}
	else if (const FParabolaGeometricTrack* Geometric = GeometricTracks.Find(TrackName))
	{
		const float CurAlpha  = FMath::Clamp(Geometric->GetAlpha(), 0.f, 1.f);
		const float NextAlpha = FMath::Clamp(CurAlpha + 0.08f, 0.f, 1.f);

		if (FMath::IsNearlyEqual(CurAlpha, NextAlpha))
			return GetOwner()->GetActorRotation();

		const FVector CurLocation  = Geometric->EvaluateAtAlpha(GetOwner(), CurAlpha);
		const FVector NextLocation = Geometric->EvaluateAtAlpha(GetOwner(), NextAlpha);

		return MakeFacingFromDir(NextLocation - CurLocation, bYawOnly, ForwardAxis);
	}

	return GetOwner()->GetActorRotation();
}

/** @brief 방향 벡터에서 회전 값을 생성한다. */
FRotator UParabolaComponent::MakeFacingFromDir(const FVector& Direction, const bool bYawOnly, const EForwardAxis ForwardAxis) const
{
	FVector Dir = Direction;

	if (bYawOnly)
		Dir.Z = 0.f;

	if (!Dir.Normalize())
		return GetOwner() ? GetOwner()->GetActorRotation() : FRotator::ZeroRotator;

	FRotator Rot;
	switch (ForwardAxis)
	{
	case EForwardAxis::X:
		Rot = FRotationMatrix::MakeFromX(Dir).Rotator();
		break;
	case EForwardAxis::Y:
		Rot = FRotationMatrix::MakeFromY(Dir).Rotator();
		break;
	case EForwardAxis::Z:
		Rot = FRotationMatrix::MakeFromZ(Dir).Rotator();
		break;
	default:
		Rot = FRotationMatrix::MakeFromX(Dir).Rotator();
		break;
	}

	if (bYawOnly)
	{
		Rot.Pitch = 0.f;
		Rot.Roll  = 0.f;
	}
	else
	{
		Rot.Roll = 0.f;
	}

	return Rot;
}

// ---- Ballistic ----
/** @brief 탄도 트랙을 등록하고 시간을 초기화한다. */
void UParabolaComponent::SetBallisticParabolaTrack(FName TrackName, const FParabolaBallisticTrack& Track)
{
	FParabolaBallisticTrack Copy = Track;
	Copy.ResetTime();
	BallisticTracks.FindOrAdd(TrackName) = Copy;
}

/** @brief 탄도 트랙의 현재 위치를 반환한다. */
FVector UParabolaComponent::GetBallisticParabolaVectorTrack(FName TrackName) const
{
	if (const FParabolaBallisticTrack* T = BallisticTracks.Find(TrackName))
	{
		return T->EvaluateAtCurrent(GetOwner());
	}
	return FVector::ZeroVector;
}

/** @brief 탄도 트랙에서 특정 알파의 위치를 계산한다. */
FVector UParabolaComponent::GetBallisticVectorAtAlphaFromTrack(FName TrackName, float Alpha) const
{
	if (const FParabolaBallisticTrack* T = BallisticTracks.Find(TrackName))
	{
		return T->EvaluateAtAlpha(GetOwner(), Alpha);
	}
	return FVector::ZeroVector;
}

/** @brief 탄도 트랙을 디버그 라인으로 그린다. */
void UParabolaComponent::DrawBallisticPath(FName TrackName, int32 NumSegments, FColor Color, float LifeTime) const
{
	if (const FParabolaBallisticTrack* T = BallisticTracks.Find(TrackName))
	{
		const float StepTime = T->Duration / NumSegments;
		FVector PrevPos = T->EvaluateAtTime(GetOwner(), 0.f);

		for (int32 i = 1; i <= NumSegments; ++i)
		{
			float t = StepTime * i;
			FVector CurrPos = T->EvaluateAtTime(GetOwner(), t);

			DrawDebugLine(
				GetWorld(),
				PrevPos,
				CurrPos,
				Color,
				false,   // 지속 여부
				LifeTime,    // 지속 시간 (초)
				0,       // Depth Priority
				1.5f     // 두께
			);

			PrevPos = CurrPos;
		}
	}
}



// ---- Geometric ----
/** @brief 기하학 트랙을 등록하고 시간을 초기화한다. */
void UParabolaComponent::SetGeometricParabolaTrack(FName TrackName, const FParabolaGeometricTrack& Track)
{
	FParabolaGeometricTrack Copy = Track;
	Copy.ResetTime();
	GeometricTracks.FindOrAdd(TrackName) = Copy;
}

/** @brief 기하학 트랙의 현재 위치를 반환한다. */
FVector UParabolaComponent::GetGeometricParabolaVectorTrack(FName TrackName) const
{
	if (const FParabolaGeometricTrack* T = GeometricTracks.Find(TrackName))
	{
		return T->EvaluateAtCurrent(GetOwner());
	}
	return FVector::ZeroVector;
}

/** @brief 기하학 트랙에서 특정 알파의 위치를 계산한다. */
FVector UParabolaComponent::GetGeometricVectorAtAlphaFromTrack(FName TrackName, float Alpha) const
{
	if (const FParabolaGeometricTrack* T = GeometricTracks.Find(TrackName))
	{
		return T->EvaluateAtAlpha(GetOwner(), Alpha);
	}
	return FVector::ZeroVector;
}

/** @brief 기하학 트랙을 디버그 라인으로 그린다. */
void UParabolaComponent::DrawGeometricPath(FName TrackName, int32 NumSegments, FColor Color, float LifeTime) const
{
	if (const FParabolaGeometricTrack* T = GeometricTracks.Find(TrackName))
	{
		const float StepAlpha = 1.f / NumSegments;
		FVector PrevPos = T->EvaluateAtAlpha(GetOwner(), 0.f);

		for (int32 i = 1; i <= NumSegments; ++i)
		{
			float a = StepAlpha * i;
			FVector CurrPos = T->EvaluateAtAlpha(GetOwner(), a);

			DrawDebugLine(
				GetWorld(),
				PrevPos,
				CurrPos,
				Color,
				false,
				LifeTime,
				0,
				1.5f
			);

			PrevPos = CurrPos;
		}
	}
}