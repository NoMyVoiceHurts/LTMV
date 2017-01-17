// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class AItemActor;

struct ItemData {
    //GENERATED_USTRUCT_BODY()
    AItemActor* actor;
    TArray<UActorComponent*> components;
};

UCLASS(config = Game)
class PROYECTO_API APlayerCharacter : public ACharacter {
    GENERATED_BODY()
public:
    /** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    float _baseTurnRate;

    /** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    float _baseLookUpRate;


    APlayerCharacter();
    virtual void BeginPlay() override;

    void ActivateScenaryItem(AItemActor* item);
    void DeactivateScenaryItem(AItemActor* item);



    void TakeLeft();




protected:
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

    /********************************** ACTION MAPPINGS ******************************************/
    /* MOVEMENT */
    void MoveForward(float Val);
    void MoveRight(float Val);
    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);

    /* SERVER */
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_TakeLeft();
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_TakeRight();
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_SaveLeft();
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_SaveRight();
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_Help();
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_Use();

    /* CLIENT */
    UFUNCTION(NetMulticast, Reliable)
    void OnRep_TakeLeft();
    UFUNCTION(NetMulticast, Reliable)
    void OnRep_TakeRight();
    UFUNCTION(NetMulticast, Reliable)
    void OnRep_SaveLeft();
    UFUNCTION(NetMulticast, Reliable)
    void OnRep_SaveRight();
    UFUNCTION(NetMulticast, Reliable)
    void OnRep_Use();
    UFUNCTION(NetMulticast, Reliable)
    void OnRep_Help();

private:
    /* REPLICATED VARIABLES */
    //UPROPERTY(ReplicatedUsing = OnRep_TakeLeft)
    //bool R_TakeLeft;
    //UPROPERTY(ReplicatedUsing = OnRep_TakeRight)
    //bool R_TakeRight;
    //UPROPERTY(ReplicatedUsing = OnRep_SaveLeft)
    //bool R_SaveLeft;
    //UPROPERTY(ReplicatedUsing = OnRep_SaveRight)
    //bool R_SaveRight;
    //UPROPERTY(ReplicatedUsing = OnRep_Help)
    //bool R_Help;
    //UPROPERTY(ReplicatedUsing = OnRep_Use)
    //bool R_Use;

    AItemActor* _itemLeft;
    AItemActor* _itemRight;
    TArray<AItemActor*> _activeScenaryItems;

    void TakeItemLeft();
    void DropItemLeft();

    void TakeItemRight();
    void DropItemRight();

    void SaveInventory(AItemActor* itemActor);

    ItemData FindItemAndComponents(const TSubclassOf<UActorComponent> ComponentClass);
};