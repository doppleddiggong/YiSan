// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UParabolaComponent.h
 * @brief EParabolaType 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "FMathHelper.h"
#include "Components/ActorComponent.h"
#include "UParabolaComponent.generated.h"

/** @brief 포물선 트랙 계산 방식을 정의한다. */
enum class EParabolaType : uint8
{
	// 초기속도·중력 사용하는 물리식임이 즉시 읽힘.
	Ballistic		UMETA(DisplayName="Physics (Ballistic)",		ToolTip="Start + V0*t + 0.5*g*t^2"),
	// 도착점 고정 + 정점높이로 기하적 아치 생성.
	Geometric		UMETA(DisplayName="Geometric (Apex Height)",	ToolTip="Lerp(Start,Target,a) + UpAxis * 4*H*a*(1-a)")
};

/** @brief 정면 방향 축을 지정한다. */
UENUM(BlueprintType)
enum class EForwardAxis : uint8 { X, Y, Z };

/** @brief 물리 기반(초기 속도/중력) 포물선을 계산하는 트랙. */
USTRUCT(BlueprintType)
struct FParabolaBallisticTrack
{
        GENERATED_BODY()

	// 입력 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Start = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.001")) float Duration = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float GravityZ = -980.f; // UE 기본 cm/s^2
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Direction = FVector(1,0,0); // 자동 정규화
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float Power = 1200.f; // cm/s

	// 평면 고정(선택)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLockZ = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bLockZ")) float LockedZ = 0.f;

	// 런타임 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient) float CurrentTime = 0.f;

        // 타임 유틸
        /** @brief 시간을 0으로 초기화한다. */
        FORCEINLINE void ResetTime() { CurrentTime = 0.f; }
        /** @brief 현재 경과 시간의 알파를 반환한다. */
        FORCEINLINE float GetAlpha() const { return FMath::Clamp(CurrentTime / FMath::Max(0.001f, Duration), 0.f, 1.f); }
        /** @brief 지정한 델타만큼 시간을 진행한다. */
        FORCEINLINE void Advance(float DeltaTime)
        {
                CurrentTime = FMath::Min(CurrentTime + FMath::Max(0.f, DeltaTime), FMath::Max(0.001f, Duration));
        }

        // 속도/평가
        /** @brief 방향/파워 조합으로부터 초기 속도를 계산한다. */
        FORCEINLINE FVector GetInitialVelocity() const
        {
                return Direction.GetSafeNormal() * Power;
        }

        /** @brief 시간값을 받아 포물선 위치를 계산한다. */
        FVector EvaluateAtTime(const AActor* /*Owner*/, float TimeSec) const
        {
		const float t = FMath::Clamp(TimeSec, 0.f, FMath::Max(0.001f, Duration));
		const FVector g(0,0,GravityZ);
		const FVector V0 = GetInitialVelocity();

		FVector P = Start + V0 * t + 0.5f * g * t * t;
		if (bLockZ)
			P.Z = LockedZ;
		return P;
	}

        /** @brief 알파값 기반 위치를 계산한다. */
        FORCEINLINE FVector EvaluateAtAlpha(const AActor* Owner, float Alpha) const
        {
                return EvaluateAtTime(Owner, Alpha * FMath::Max(0.001f, Duration));
        }

        /** @brief 현재 시간 기준 위치를 계산한다. */
        FORCEINLINE FVector EvaluateAtCurrent(const AActor* Owner) const
        {
                return EvaluateAtTime(Owner, CurrentTime);
        }

        // 목표점+시간으로 V0 역산 → Direction/Power 세팅
        /** @brief 목표 위치와 비행 시간으로 초기 속도를 역산한다. */
        static FVector SolveV0ForArc(const FVector& InStart, const FVector& InEnd, float t, float InGravityZ=-980.f)
        {
                return FMathHelper::SolveV0ForProjectile( InStart, InEnd, t, InGravityZ );
        }

        /** @brief 속도를 방향과 크기로 분해한다. */
        static void SplitVelocity(const FVector& V0, FVector& OutDir, float& OutPow)
        {
                OutPow = V0.Length();
                OutDir = (OutPow > SMALL_NUMBER) ? V0 / OutPow : FVector(1,0,0);
        }

        /** @brief 시작/도착점과 시간으로 트랙 파라미터를 초기화한다. */
        void ApplyArcSolution(const FVector& InStart, const FVector& InEnd, float t)
        {
                Start = InStart;
                Duration = FMath::Max(0.001f, t);
                const FVector V0 = SolveV0ForArc(InStart, InEnd, Duration, GravityZ);
                SplitVelocity(V0, Direction, Power);
                ResetTime();
        }
};

/** @brief 정점 높이를 지정해 기하학적 포물선을 계산하는 트랙. */
USTRUCT(BlueprintType)
struct FParabolaGeometricTrack
{
        GENERATED_BODY()

	// 입력 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Start = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Target = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.001")) float Duration = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0")) float ApexHeight = 200.f; // 정점에서 +H
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector UpAxis = FVector::UpVector;

	// 평면 고정(선택)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLockZ = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bLockZ")) float LockedZ = 0.f;

	// 런타임 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient) float CurrentTime = 0.f;

        // 타임 유틸
        /** @brief 시간을 0으로 초기화한다. */
        FORCEINLINE void ResetTime() { CurrentTime = 0.f; }
        /** @brief 현재 경과 시간의 알파를 반환한다. */
        FORCEINLINE float GetAlpha() const { return FMath::Clamp(CurrentTime / FMath::Max(0.001f, Duration), 0.f, 1.f); }
        /** @brief 지정한 델타만큼 시간을 진행한다. */
        FORCEINLINE void Advance(float DeltaTime)
        {
                CurrentTime = FMath::Min(CurrentTime + FMath::Max(0.f, DeltaTime), FMath::Max(0.001f, Duration));
        }

        // 평가(직선보간 + 4Ha(1-a) 버프)
        /** @brief 시간값을 받아 포물선 위치를 계산한다. */
        FVector EvaluateAtTime(const AActor* /*Owner*/, float TimeSec) const
        {
		const float D = FMath::Max(0.001f, Duration);
		const float t = FMath::Clamp(TimeSec, 0.f, D);
		const float a = t / D;

		return FMathHelper::InterpArcSin(Start, Target, ApexHeight, a);
	}

        /** @brief 알파값 기반 위치를 계산한다. */
        FORCEINLINE FVector EvaluateAtAlpha(const AActor* Owner, float Alpha) const
        {
                return EvaluateAtTime(Owner, Alpha * FMath::Max(0.001f, Duration));
        }

        /** @brief 현재 시간 기준 위치를 계산한다. */
        FORCEINLINE FVector EvaluateAtCurrent(const AActor* Owner) const
        {
                return EvaluateAtTime(Owner, CurrentTime);
        }

        /** @brief 시작/도착점과 정점 높이로 트랙 파라미터를 초기화한다. */
        void ApplyArcSolution(const FVector& InStart, const FVector& InTarget, const float InApexHeight, float t, const FVector& InUpAxis = FVector::UpVector)
        {
                Start = InStart;
                Target = InTarget;
                ApexHeight = FMath::Max(0.f, InApexHeight);
                UpAxis = InUpAxis;
                Duration = FMath::Max(0.001f, t);
                ResetTime();
        }
};


/**
 * @brief 포물선 이동을 계산해 위치/회전을 제공하는 컴포넌트.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COFFEELIBRARY_API UParabolaComponent : public UActorComponent
{
        GENERATED_BODY()

public:
        UParabolaComponent();

protected:
        /** @brief 초기화 시 기본 설정을 수행한다. */
        virtual void BeginPlay() override;

public:
        /** @brief 매 프레임 트랙을 업데이트한다. */
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
        /** @brief 등록된 모든 트랙을 수동 업데이트한다. */
        void UpdateTracks(float DeltaTime);

        /** @brief 트랙 진행 방향에 맞는 회전을 계산한다. */
        UFUNCTION(BlueprintCallable, Category="Parabola")
        FRotator GetParabolaFacing(FName TrackName, bool bYawOnly, EForwardAxis ForwardAxis) const;

        /** @brief 지정된 방향 벡터에서 회전을 생성한다. */
        UFUNCTION(BlueprintCallable, Category="Parabola")
        FRotator MakeFacingFromDir(const FVector& Direction, const bool bYawOnly, const EForwardAxis ForwardAxis) const;

        // --- Ballistic ---
        /** @brief 탄도 포물선 트랙을 설정한다. */
        UFUNCTION(BlueprintCallable, Category="Parabola|Ballistic")
        void SetBallisticParabolaTrack(FName TrackName, const FParabolaBallisticTrack& Track);

        /** @brief 탄도 트랙의 현재 위치를 반환한다. */
        UFUNCTION(BlueprintPure, Category="Parabola|Ballistic")
        FVector GetBallisticParabolaVectorTrack(FName TrackName) const;

        /** @brief 탄도 트랙에서 특정 알파 위치를 반환한다. */
        UFUNCTION(BlueprintPure, Category="Parabola|Ballistic")
        FVector GetBallisticVectorAtAlphaFromTrack(FName TrackName, float Alpha) const;

        /** @brief 탄도 경로를 디버그 드로우한다. */
        UFUNCTION(BlueprintCallable, Category="Parabola|Ballistic")
        void DrawBallisticPath(FName TrackName, int32 NumSegments, FColor Color, float LifeTime) const;

        // --- Geometric ---
        /** @brief 기하학 포물선 트랙을 설정한다. */
        UFUNCTION(BlueprintCallable, Category="Parabola|Geometric")
        void SetGeometricParabolaTrack(FName TrackName, const FParabolaGeometricTrack& Track);

        /** @brief 기하학 트랙의 현재 위치를 반환한다. */
        UFUNCTION(BlueprintPure, Category="Parabola|Geometric")
        FVector GetGeometricParabolaVectorTrack(FName TrackName) const;

        /** @brief 기하학 트랙에서 특정 알파 위치를 반환한다. */
        UFUNCTION(BlueprintPure, Category="Parabola|Geometric")
        FVector GetGeometricVectorAtAlphaFromTrack(FName TrackName, float Alpha) const;

        /** @brief 기하학 경로를 디버그 드로우한다. */
        UFUNCTION(BlueprintCallable, Category="Parabola|Geometric")
        void DrawGeometricPath(FName TrackName, int32 NumSegments, FColor Color, float LifeTime) const;

private:
	TMap<FName, FParabolaBallisticTrack> BallisticTracks;
	TMap<FName, FParabolaGeometricTrack> GeometricTracks;
};
