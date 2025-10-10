// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGate.generated.h"

UCLASS()
class YISAN_API AGate : public AActor
{
    GENERATED_BODY()

public:
    AGate();

    UFUNCTION(BlueprintCallable, Category = "Gate")
    void OpenDoor();

    UFUNCTION(BlueprintCallable, Category = "Gate")
    void CloseDoor();

protected:
    virtual void BeginPlay() override;
    
    UFUNCTION()
    void OnDoorMessage(int32 InGateID, bool Open);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Gate")
    void PlayOpenDoorAnimation();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Gate")
    void PlayCloseDoorAnimation();

    bool IsPlayerActor(const AActor* OtherActor) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class USceneComponent> RootComp;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UStaticMeshComponent> Door_Left;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UStaticMeshComponent> Door_Right;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UBoxComponent> BoxCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate")
    int32 GateID;

private:
    UPROPERTY(VisibleInstanceOnly, Category = "Gate")
    bool bIsOpen;

    UPROPERTY()
    TArray<TObjectPtr<class APawn>> OverlappingPawns;
};
