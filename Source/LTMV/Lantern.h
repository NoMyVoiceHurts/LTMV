// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ItfUsableItem.h"
#include "FMODAudioComponent.h"

#include "Components/SceneComponent.h"
#include "Lantern.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LTMV_API ULantern : public USceneComponent, public IItfUsableItem {
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULantern();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Lantern Component")
    float _batteryLife;

	UFMODAudioComponent* _LanternClickAudio;

    float GetBatteryLife();
    void AddBatteryLife(float BatteryAmount);
    void UsingBattery();
    void PowerOff();
    void PowerOn();

    /*Inerfaces*/
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Use Item")
    void UseItemPressed();
    virtual void UseItemPressed_Implementation() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Use Item")
    void UseItemReleased();
    virtual void UseItemReleased_Implementation() override;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    bool _isLanternOn;
};
