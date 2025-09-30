// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#pragma once

#include "CoreMinimal.h"
#include "AGameCharacter.h"
#include "IControllable.h"
#include "APlayerActor.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Dopple))
class COFFEELIBRARY_API APlayerActor : public AGameCharacter, public IControllable
{
	GENERATED_BODY()

public:
	APlayerActor();

protected:
	virtual void BeginPlay() override;


public:

	virtual void Landed(const FHitResult& Hit) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	TObjectPtr<class UCameraShakeSystem> CameraShakeSystem;

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
