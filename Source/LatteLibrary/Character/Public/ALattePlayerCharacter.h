// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file APlayerActor.h
 * @brief Declares the player-controlled character actor.
 */

#pragma once

#include "CoreMinimal.h"
#include "ALatteGameCharacter.h"
#include "IControllable.h"
#include "ALattePlayerCharacter.generated.h"

/**
 * @brief Main character driven directly by the player.
 * @details Extends AGameCharacter with player-specific input and camera handling.
 * @ingroup Character
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple))
class LATTELIBRARY_API ALattePlayerCharacter : public ALatteGameCharacter, public IControllable
{
	GENERATED_BODY()

public:
	ALattePlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual void Landed(const FHitResult& Hit) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	TObjectPtr<class UCameraShakeSystem> CameraShakeSystem;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

public: // Control Interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Move(const FVector2D& Axis) override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Look(const FVector2D& Axis) override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_AltitudeUp() override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_AltitudeDown() override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_AltitudeReleased() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Jump() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Landing() override;
};
