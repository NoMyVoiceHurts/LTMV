// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#ifndef __PLAYER_H
#include "PlayerCharacter.h"
#define __PLAYER_H
#endif

#include "Components/ActorComponent.h"
#include "ItemOverlap.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROYECTO_API UItemOverlap : public UActorComponent {
    GENERATED_BODY()

public:
    UItemOverlap();

    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    virtual void activateItem(UPrimitiveComponent* OverlappedComp,
                              APlayerCharacter* OtherActor,
                              UPrimitiveComponent* OtherComp,
                              int32 OtherBodyIndex, bool bFromSweep,
                              const FHitResult& SweepResult);

    virtual void deactivateItem(UPrimitiveComponent* OverlappedComp, APlayerCharacter* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
protected:
    bool _isActive;
    AActor* _actor;
};