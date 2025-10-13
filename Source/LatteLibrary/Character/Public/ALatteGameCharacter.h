// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EBodyPartType.h"
#include "ECharacterType.h"
#include "UStatSystem.h"
#include "ALatteGameCharacter.generated.h"

/**
 * @file AGameCharacter.h
 * @brief LatteLibrary 전반에서 사용하는 기반 캐릭터 클래스를 선언합니다.
 */

/**
 * @brief 전투, 능력치, 애니메이션 헬퍼를 연결하는 기반 캐릭터 구현입니다.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple))
class LATTELIBRARY_API ALatteGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
    /** @brief 기본 구성 요소와 서브 시스템을 초기화합니다. */
    ALatteGameCharacter();

protected:
    /** @brief 게임 시작 시 서브 시스템을 연결합니다. */
    virtual void BeginPlay() override;

    /** @brief 플레이 종료 시 리소스를 해제합니다. */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
    /** @brief 몽타주 알림 처리 델리게이트를 바인딩합니다. */
    void BindMontageDelegates(UAnimInstance* Anim);

    /** @brief 몽타주 알림 델리게이트 연결을 해제합니다. */
    void UnbindMontageDelegates(UAnimInstance* Anim);

	UFUNCTION()
    /** @brief 몽타주 노티파이 시작 시 호출되는 콜백입니다. */
    void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
	
public:
	FORCEINLINE UArrowComponent* GetBodyPart(EBodyPartType Part) const
	{
		switch (Part)
		{
		case EBodyPartType::Hand_L: return LeftHandComp;
		case EBodyPartType::Hand_R: return RightHandComp;
		case EBodyPartType::Foot_L: return LeftFootComp;
		case EBodyPartType::Foot_R: return RightFootComp;
		default:	return LeftHandComp;
		}
	}


	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsCombatStart() const
	{
		return bIsCombatStart;
	};

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsCombatResult() const
	{
		return bIsCombatResult;
	};

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsPlayer() const
	{
		return StatSystem->IsPlayer();
	};

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsEnemy() const
	{
		return StatSystem->IsPlayer() == false;
	};

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE float CurHP(float Per = 1.0f)
	{
		return StatSystem->GetCurHP() * Per;
	}
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE float MaxHP(float Per = 1.0f)
	{
		return StatSystem->GetMaxHP() * Per;
	}
	
	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsWinner() const
	{
		return bIsWinner;
	};

	UFUNCTION(BlueprintCallable, Category="CharacterType")
	FORCEINLINE ECharacterType GetCharacterType()
	{
		return CharacterType;
	}
	

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsHolding() const
	{
		return bIsHold;
	};

	UFUNCTION(BlueprintCallable, Category="GameState")
	FORCEINLINE void SetHold(const bool bState)
	{
		this->bIsHold = bState;
	};
		
public:
	UFUNCTION(BlueprintCallable, Category="Setup")
	void SetupCharacterFromType(const ECharacterType Type, const bool bIsAnother);
	
	/**
	 * @brief 캐릭터의 조작 가능 상태를 확인합니다.
	 * @details 캐릭터가 피격, 사망, 또는 다른 행동으로 인해 조종 불가능한 상태인지 확인합니다.
	 * @return 조작 가능하면 true, 아니면 false를 반환합니다.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsControlEnable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsMoveEnable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsAttackEnable();


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsDead();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsHitting();
	UFUNCTION(BlueprintPure, Category="Character|Sight")
	bool IsInSight(const AActor* Other) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void OnLookTarget();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void OnFlyEnd();
	
	UFUNCTION(BlueprintCallable, Category="Fly")
	void SetFlying();
	UFUNCTION(BlueprintCallable, Category="Fly")
	void SetFallingToWalk();

	UFUNCTION(BlueprintCallable, Category="Command")
	void RecoveryMovementMode(const EMovementMode InMovementMode);
	
	UFUNCTION(BlueprintCallable, Category="Stat")
	FORCEINLINE float GetAttackDamage()
	{
		return StatSystem->GetAttackDamage();
	}

public:
	UFUNCTION(BlueprintCallable, Category="Montage")
	FORCEINLINE UAnimInstance* GetAnimInstance()
	{
		return AnimInstance;
	}
	

	
	UFUNCTION(BlueprintCallable, Category="Montage")
	void PlayTypeMontage(const EMontageType Type);

	UFUNCTION(BlueprintCallable, Category="Montage")
	void PlayTargetMontage(UAnimMontage* AnimMontage);

	UFUNCTION(BlueprintCallable, Category="Montage")
	void StopTargetMontage(const EMontageType Type, const float BlendInOutTime);
	
public: // Combat Character ShaderComp
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UStatSystem> StatSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UHitStopSystem> HitStopSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UKnockbackSystem> KnockbackSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UFlySystem> FlySystem;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
	TObjectPtr<class ALatteGameCharacter> TargetActor;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	TObjectPtr<class USkeletalMeshComponent> MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	TObjectPtr<class UCharacterMovementComponent> MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	TObjectPtr<class UAnimInstance> AnimInstance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	TObjectPtr<class UArrowComponent> LeftHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	TObjectPtr<class UArrowComponent> RightHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	TObjectPtr<class UArrowComponent> LeftFootComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	TObjectPtr<class UArrowComponent> RightFootComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Sight", meta=(ClampMin="0"))
	float SightRange = 1200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Sight", meta=(ClampMin="0", ClampMax="180"))
	float SightHalfFOVDeg = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Trace")
	float TraceLength  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Trace")
	float TraceRadius  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Trace")
	float TraceDrawTime = 1.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|CombatState")
	bool bIsCombatStart = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|CombatState")
	bool bIsCombatResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|CombatState")
	bool bIsWinner = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
	bool IsHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character")
	bool bIsHold = false;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Data")
	TObjectPtr<class UCharacterData> CharacterData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TArray<TObjectPtr<UAnimMontage>> HitMontages;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> DeathMontage;
	
protected:
	bool bDelegatesBound = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECharacterType CharacterType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EMovementMode> PrevMoveMode;
};
