// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Menu3D.generated.h"


class UMenuPanel;
class UInputMenu;

UCLASS()
class LTMV_API AMenu3D : public AActor {
	GENERATED_BODY()
	
public:
    bool _IsMenuHidden;

    AMenu3D(const class FObjectInitializer& OI);
    void ToogleMenu(FVector Location, FRotator Rotation);

    void AddSubmenu(UMenuPanel* Submenu);
    void SetSubmenuByIndex(int Index);

protected:
    FVector _SubmenuLocation = FVector(0, 0, 20);
    FRotator _SubmenuRotator = FRotator(0, 180, 0);

    /*** DECORATORS ***/
    UPROPERTY(Category = "Menu Decorator", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* _TopDecorator;
    UPROPERTY(Category = "Menu Decorator", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* _BottomDecorator;
    UPROPERTY(Category = "Menu Decorator", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* _MiddleDecorator;

    UPROPERTY(Category = "Menu Decorator", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UInputMenu* _BackSubmenu;

    void EnableBackSubmenu(bool Enable);

    /*** PANELS ***/
    TArray<UMenuPanel*> _Submenus;

    /* BINDINGS */
    void OnButtonBack(UInputMenu* InputMenu);

private:
    TArray<int> _Breadcrumb;
};
