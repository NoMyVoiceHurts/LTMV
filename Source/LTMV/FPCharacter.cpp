// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "FPCharacter.h"

#include "Inventory.h"
#include "InventoryItem.h"

#include "ItfUsable.h"
#include "ItfUsableItem.h"
#include "HandPickItem.h"
#include "Components/WidgetInteractionComponent.h"

AFPCharacter::AFPCharacter() : Super() {
    _Inventory = CreateDefaultSubobject<UInventory>(TEXT("Inventory"));
    /*RAYCAST PARAMETERS*/
    _RayParameter = 250.0f;

    _PlayerCamera->bUsePawnControlRotation = true;
    _PlayerCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("FPVCamera"));
    _WidgetInteractionComp->AttachToComponent(_PlayerCamera, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInput) {
    Super::SetupPlayerInputComponent(PlayerInput);

    /* MOVEMENT */
    PlayerInput->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInput->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    PlayerInput->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInput->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
    PlayerInput->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInput->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

    /* ACTIONS */
    PlayerInput->BindAction("TakeDropRight", IE_Released, this, &AFPCharacter::TakeDropRight);
    PlayerInput->BindAction("TakeDropLeft", IE_Released, this, &AFPCharacter::TakeDropLeft);

    PlayerInput->BindAction("Use", IE_Pressed, this, &AFPCharacter::UsePressed);
    PlayerInput->BindAction("Use", IE_Released, this, &AFPCharacter::UseReleased);
}

void AFPCharacter::BeginPlay() {
    Super::BeginPlay();
}

void AFPCharacter::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    //_StepsAudioComp->SetParameter(FName("humedad"), 0.9);
    Raycasting();
}

FHitResult AFPCharacter::Raycasting() {
    bool bHitRayCastFlag = false;
    FCollisionQueryParams CollisionInfo;

    FVector StartRaycast = _PlayerCamera->GetComponentLocation();
    FVector EndRaycast = _PlayerCamera->GetForwardVector() * _RayParameter + StartRaycast;

    bHitRayCastFlag = GetWorld()->LineTraceSingleByChannel(hitResult, StartRaycast, EndRaycast, ECC_Visibility, CollisionInfo);
    //DrawDebugLine(GetWorld(), StartRaycast, EndRaycast, FColor(255, 0, 0), false, -1.0f, (uint8)'\000', 0.8f);

    if (bHitRayCastFlag && hitResult.Actor.IsValid()) {
        //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You hit: %s"), *hitResult.Actor->GetName()));
        UActorComponent* actorComponent = hitResult.GetComponent();

        TArray<UActorComponent*> components = actorComponent->GetOwner()->GetComponentsByClass(UActorComponent::StaticClass());

        for (UActorComponent* component : components) {

            //Highlight outline colors:
            //GREEN: 252 | BLUE: 253 | ORANGE: 254 | WHITE: 255
            if (component->GetClass()->ImplementsInterface(UItfUsable::StaticClass())) {
                lastMeshFocused = Cast<UStaticMeshComponent>(component->GetOwner()->GetComponentByClass(
                    UStaticMeshComponent::StaticClass()));

                lastMeshFocused->SetRenderCustomDepth(true);
                lastMeshFocused->SetCustomDepthStencilValue(252);
                bInventoryItemHit = true;
            }
            else if (component->GetClass() == UInventoryItem::StaticClass()) {
                lastMeshFocused = Cast<UStaticMeshComponent>(component->GetOwner()->GetComponentByClass(
                    UStaticMeshComponent::StaticClass()));

                lastMeshFocused->SetRenderCustomDepth(true);
                lastMeshFocused->SetCustomDepthStencilValue(253);
                bInventoryItemHit = true;
            }
            else if (component->GetClass() == UHandPickItem::StaticClass()) {
                lastMeshFocused = Cast<UStaticMeshComponent>(component->GetOwner()->GetComponentByClass(
                    UStaticMeshComponent::StaticClass()));

                lastMeshFocused->SetRenderCustomDepth(true);
                lastMeshFocused->SetCustomDepthStencilValue(255);
                bInventoryItemHit = true;
            }

            //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("You hit: %s"), *hitResult.Actor->GetName()));
        }
    }

    //If Raycast is not hitting any actor, disable the outline
    if (bInventoryItemHit && hitResult.Actor != lastMeshFocused->GetOwner()) {

        lastMeshFocused->SetCustomDepthStencilValue(0);
        lastMeshFocused->SetRenderCustomDepth(false);

        bInventoryItemHit = false;
    }


    return hitResult;
}

/****************************************** ACTION MAPPINGS **************************************/
/************** USE *************/
void AFPCharacter::UsePressed() {
    /* RAYCASTING DETECTION */
    if (hitResult.GetActor()) {
        TArray<UActorComponent*> Components;
        hitResult.GetActor()->GetComponents(Components);

        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsable::StaticClass())) {
                SERVER_UsePressed(Component);
            }
        }
    }
}

void AFPCharacter::UseReleased() {
    /* RAYCASTING DETECTION */
    if (hitResult.GetActor()) {
        TArray<UActorComponent*> Components;
        hitResult.GetActor()->GetComponents(Components);

        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsable::StaticClass())) {
                SERVER_UseReleased(Component);
            }
        }
    }
}

/******** USE ITEM LEFT *********/
void AFPCharacter::UseLeftPressed() {
    if (_ItemLeft) {
        TArray<UActorComponent*> Components;
        _ItemLeft->GetComponents(Components);

        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsableItem::StaticClass())) {
                IItfUsableItem* ItfObject = Cast<IItfUsableItem>(Component);
                if (ItfObject) ItfObject->Execute_UseItemPressed(Component);
            }
        }
    }
}

void AFPCharacter::UseLeftReleased() {
    if (_ItemLeft) {
        TArray<UActorComponent*> Components;
        _ItemLeft->GetComponents(Components);

        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsableItem::StaticClass())) {
                IItfUsableItem* ItfObject = Cast<IItfUsableItem>(Component);
                if (ItfObject) ItfObject->Execute_UseItemReleased(Component);
            }
        }
    }
}

/******* USE ITEM RIGHT *********/
void AFPCharacter::UseRightPressed() {
    if (_ItemRight) {
        TArray<UActorComponent*> Components;
        _ItemRight->GetComponents(Components);

        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsableItem::StaticClass())) {
                IItfUsableItem* ItfObject = Cast<IItfUsableItem>(Component);
                if (ItfObject) ItfObject->Execute_UseItemPressed(Component);
            }
        }
    }
}

void AFPCharacter::UseRightReleased() {
    if (_ItemRight) {
        TArray<UActorComponent*> Components;
        _ItemRight->GetComponents(Components);

        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsableItem::StaticClass())) {
                IItfUsableItem* ItfObject = Cast<IItfUsableItem>(Component);
                if (ItfObject) ItfObject->Execute_UseItemReleased(Component);
            }
        }
    }
}

/********** TAKE & DROP RIGHT HAND ***********/
void AFPCharacter::TakeDropRight() {
    AActor* ActorFocused = GetItemFocused();
    if (ActorFocused) {
        if (ActorFocused->GetComponentByClass(UInventoryItem::StaticClass())) {
            /* Save scenary inventory item */
            SERVER_SaveItemInventory(ActorFocused);
        }
        else if (ActorFocused->GetComponentByClass(UHandPickItem::StaticClass())) {
            if (_ItemRight && _ItemRight->GetComponentByClass(UHandPickItem::StaticClass())) {
                /* Replace item */
                SERVER_DropRight();
                SERVER_TakeRight(ActorFocused);
            }
            else if (_ItemRight && _ItemRight->GetComponentByClass(UInventoryItem::StaticClass())) {
                /* Save hand inventory item */
                SERVER_SaveItemInventoryRight();
            }
            else if (!_ItemRight) {
                /* Take item */
                SERVER_TakeRight(ActorFocused);
            }
        }
    }
    else if (_ItemRight && _ItemRight->GetComponentByClass(UHandPickItem::StaticClass())) {
        /* Drop item */
        SERVER_DropRight();
    }
    else if (_ItemRight && _ItemRight->GetComponentByClass(UInventoryItem::StaticClass())) {
        /* Save hand inventory item */
        SERVER_SaveItemInventoryRight();
    }
}

/********** TAKE & DROP LEFT HAND ***********/
void AFPCharacter::TakeDropLeft() {
    AActor* ActorFocused = GetItemFocused();
    if (ActorFocused) {
        if (ActorFocused->GetComponentByClass(UInventoryItem::StaticClass())) {
            /* Save scenary inventory item */
            SERVER_SaveItemInventory(ActorFocused);
        }
        else if (ActorFocused->GetComponentByClass(UHandPickItem::StaticClass())) {
            if (_ItemLeft && _ItemLeft->GetComponentByClass(UHandPickItem::StaticClass())) {
                /* Replace item */
                SERVER_DropLeft();
                SERVER_TakeLeft(ActorFocused);
            }
            else if (_ItemLeft && _ItemLeft->GetComponentByClass(UInventoryItem::StaticClass())) {
                /* Save hand inventory item */
                SERVER_SaveItemInventoryLeft();
            }
            else if (!_ItemLeft) {
                /* Take item */
                SERVER_TakeLeft(ActorFocused);
            }
        }
    }
    else if (_ItemLeft && _ItemLeft->GetComponentByClass(UHandPickItem::StaticClass())) {
        /* Drop item */
        SERVER_DropLeft();
    }
    else if (_ItemLeft && _ItemLeft->GetComponentByClass(UInventoryItem::StaticClass())) {
        /* Save hand inventory item */
        SERVER_SaveItemInventoryLeft();
    }
}

/**************************************** INVENTORY **********************************************/
bool AFPCharacter::SERVER_SaveItemInventory_Validate(AActor* Actor) { return true; }
void AFPCharacter::SERVER_SaveItemInventory_Implementation(AActor* Actor) {
    MULTI_SaveItemInventory(Actor);
}
void AFPCharacter::MULTI_SaveItemInventory_Implementation(AActor* Actor) {
    UStaticMeshComponent* ItemMesh = Cast<UStaticMeshComponent>(Actor->GetComponentByClass(
        UStaticMeshComponent::StaticClass()));
    if (ItemMesh) {
        ItemMesh->SetMobility(EComponentMobility::Movable);
        _Inventory->AddItem(Actor);
    }
}

bool AFPCharacter::SERVER_SaveItemInventoryLeft_Validate() { return true; }
void AFPCharacter::SERVER_SaveItemInventoryLeft_Implementation() {
    MULTI_SaveItemInventoryLeft();
}
void AFPCharacter::MULTI_SaveItemInventoryLeft_Implementation() {
    UStaticMeshComponent* ItemMesh = Cast<UStaticMeshComponent>(_ItemLeft->GetComponentByClass(
        UStaticMeshComponent::StaticClass()));
    if (ItemMesh) {
        ItemMesh->SetMobility(EComponentMobility::Movable);
        _Inventory->AddItem(_ItemLeft);
        _ItemLeft = nullptr;
    }
}

bool AFPCharacter::SERVER_SaveItemInventoryRight_Validate() { return true; }
void AFPCharacter::SERVER_SaveItemInventoryRight_Implementation() {
    MULTI_SaveItemInventoryRight();
}
void AFPCharacter::MULTI_SaveItemInventoryRight_Implementation() {
    UStaticMeshComponent* ItemMesh = Cast<UStaticMeshComponent>(_ItemRight->GetComponentByClass(
        UStaticMeshComponent::StaticClass()));
    if (ItemMesh) {
        ItemMesh->SetMobility(EComponentMobility::Movable);
        _Inventory->AddItem(_ItemRight);
        _ItemRight = nullptr;
    }
}

void AFPCharacter::PickItemInventory(AActor* ItemActor, FKey KeyStruct) {
    SERVER_PickItemInventory(ItemActor, KeyStruct);
}

bool AFPCharacter::SERVER_PickItemInventory_Validate(AActor* ItemActor, FKey KeyStruct) { return true; }
void AFPCharacter::SERVER_PickItemInventory_Implementation(AActor* ItemActor, FKey KeyStruct) {
    MULTI_PickItemInventory(ItemActor, KeyStruct);
}

void AFPCharacter::MULTI_PickItemInventory_Implementation(AActor* ItemActor, FKey KeyStruct) {
    /*Obtaining item components*/
    if (ItemActor) {
        UStaticMeshComponent* ItemMesh = Cast<UStaticMeshComponent>(ItemActor->GetComponentByClass(
            UStaticMeshComponent::StaticClass()));
        UInventoryItem* InventoryItemComp = Cast<UInventoryItem>(ItemActor->GetComponentByClass(
            UInventoryItem::StaticClass()));
        if (ItemMesh && InventoryItemComp) {
            ItemMesh->SetMobility(EComponentMobility::Movable);
            ItemMesh->SetSimulatePhysics(false);

            if (KeyStruct == EKeys::LeftMouseButton) {
                if (_ItemLeft && _ItemLeft->GetComponentByClass(UInventoryItem::StaticClass())) {
                    /* Save hand inventory item */
                    SERVER_SaveItemInventoryLeft();
                }
                else if (_ItemLeft && _ItemLeft->GetComponentByClass(UHandPickItem::StaticClass())) {
                    /* Drop item */
                    SERVER_DropLeft();
                }
                ItemMesh->AttachToComponent(GetMesh(),
                                            FAttachmentTransformRules::KeepRelativeTransform,
                                            TEXT("itemHand_l"));

                ItemMesh->RelativeLocation = InventoryItemComp->_locationAttachFromInventory_L;
                ItemMesh->RelativeRotation = InventoryItemComp->_rotationAttachFromInventory_L;
                ItemMesh->GetOwner()->SetActorHiddenInGame(false);

                _ItemLeft = ItemActor;
                AddRadioDelegates(ItemActor);

                /*If the item is equipped in the other hand*/
                if (_ItemRight && _ItemRight == ItemActor) {
                    _ItemRight = nullptr;
                }
            }
            else if (KeyStruct == EKeys::RightMouseButton) {
                if (_ItemRight && _ItemRight->GetComponentByClass(UInventoryItem::StaticClass())) {
                    /* Save hand inventory item */
                    SERVER_SaveItemInventoryRight();
                }
                else if (_ItemRight && _ItemRight->GetComponentByClass(UHandPickItem::StaticClass())) {
                    /* Drop item */
                    SERVER_DropRight();
                }
                ItemMesh->AttachToComponent(GetMesh(),
                                            FAttachmentTransformRules::KeepRelativeTransform,
                                            TEXT("itemHand_r"));

                ItemMesh->RelativeLocation = InventoryItemComp->_locationAttachFromInventory_L;
                ItemMesh->RelativeRotation = InventoryItemComp->_rotationAttachFromInventory_L;
                ItemMesh->GetOwner()->SetActorHiddenInGame(false);

                _ItemRight = ItemActor;
                AddRadioDelegates(ItemActor);

                /*If the item is equipped in the other hand*/
                if (_ItemLeft && _ItemLeft == ItemActor) {
                    _ItemLeft = nullptr;
                }
            }
        }
    }
}

/****************************************** AUXILIAR FUNCTIONS ***********************************/
UTexture2D* AFPCharacter::GetItemAt(int itemIndex) {
    return _Inventory->GetItemAt(itemIndex);
}

AActor* AFPCharacter::GetItemFocused() {
    AActor* ActorFocused = hitResult.GetActor();
    if (ActorFocused && ActorFocused->GetComponentByClass(UStaticMeshComponent::StaticClass()) &&
        (ActorFocused->GetComponentByClass(UInventoryItem::StaticClass()) ||
         ActorFocused->GetComponentByClass(UHandPickItem::StaticClass()))) {
        return ActorFocused;
    }
    return nullptr;
}