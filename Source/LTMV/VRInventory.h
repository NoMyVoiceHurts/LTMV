// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "VRInventory.generated.h"

UCLASS()
class LTMV_API AVRInventory : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRInventory();

    bool bIsVRInventoryHidden;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UWidgetComponent* CanvasWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
    TSubclassOf<UUserWidget> CanvasWidgetSubclass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
    TSubclassOf<UUserWidget> SlotWidgetSubclass1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
    TSubclassOf<UUserWidget> SlotWidgetSubclass2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
    TSubclassOf<UUserWidget> SlotWidgetSubclass3;

    void ToggleVRInventory(FVector Location, FRotator Rotation);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UWidgetComponent* InitWidgetComp(UWidgetComponent* WidgetComponent);
};
