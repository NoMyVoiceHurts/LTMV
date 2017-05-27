// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "InputMenu.h"


UInputMenu::UInputMenu(const FObjectInitializer& OI) : Super(OI) {
    PrimaryComponentTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(
        TEXT("StaticMesh'/Game/Art/Common/Menu/Meshes/menu2_cajaprueba.menu2_cajaprueba'"));
    SetStaticMesh(Finder.Object);

    _Color = FColor::FromHex("293E3DFF"); 
    _HoverColor = FColor::FromHex("A8FFFAFF");

    _TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("_TextRender"));
    _TextRender->SetWorldSize(12);
    _TextRender->SetTextRenderColor(_Color);
    _TextRender->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    _TextRender->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);

    _NextPoint = FVector();
    bool _IsFlee = false;
    _IsLoading = false;

    //OnComponentActivated.AddDynamic(this, &UInputMenu::OnActivate);
    //OnComponentDeactivated.AddDynamic(this, &UInputMenu::OnDeactivate);
}

//void UInputMenu::OnActivate(UActorComponent* Component, bool bReset) {
//    ULibraryUtils::Log("OnActivate");
//}
//
//void UInputMenu::OnDeactivate(UActorComponent* Component) {
//    ULibraryUtils::Log("OnDeactivate");
//}

void UInputMenu::BeginPlay() {
    _TextRender->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform,
                                   FName("SocketText"));
    _TextRender->SetText(FText::FromString(GetFName().ToString()));
    _TextRender->RegisterComponent();
}

void UInputMenu::Init(const FVector MenuPanelLocation) {
    _InitialLocation = MenuPanelLocation;
    RelativeLocation = _InitialLocation;

    UpdateNextPoint();
    bool _IsFlee = true;
}

void UInputMenu::TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (FVector::Dist(RelativeLocation, _NextPoint) <= 3) {
        if (_IsFlee) _NextPoint = _InitialLocation;
        else UpdateNextPoint();
        _IsFlee = !_IsFlee;
    }
    AddRelativeLocation((_NextPoint - RelativeLocation).GetSafeNormal() * 1.5 * DeltaTime);

    /* LOADING */
    if (_IsLoading) AddRelativeRotation(FRotator(0, 150 * DeltaTime, 0));
}

void UInputMenu::UpdateNextPoint() {
    _NextPoint.X = _InitialLocation.X + FMath::FRandRange(-5, 5);
    _NextPoint.Y = _InitialLocation.Y + FMath::FRandRange(-5, 5);
    _NextPoint.Z = _InitialLocation.Z + FMath::FRandRange(-5, 5);
}

void UInputMenu::PressEvents() {
    if (!_IsLoading) {
        EndhoverInteraction();
        _InputMenuPressedEvent.Broadcast(this);
    }
}

void UInputMenu::ReleaseEvents() {
    if (!_IsLoading) {
        HoverInteraction();
        _InputMenuReleasedEvent.Broadcast(this);
    }
}

void UInputMenu::HoverInteraction() {
    if (_TextRender) _TextRender->SetTextRenderColor(_HoverColor);
}

void UInputMenu::EndhoverInteraction() {
    if (_TextRender) _TextRender->SetTextRenderColor(_Color);
}

void UInputMenu::Enable(bool Enable) {
    SetActive(Enable);
    SetHiddenInGame(!Enable, true);
    SetComponentTickEnabled(Enable);
    SetVisibility(Enable, true);
    if (Enable) SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    else SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UInputMenu::SetLoading(bool IsLoading, FString Text) {
    _IsLoading = IsLoading;
    if (_IsLoading) {
        _PrevText = _TextRender->Text;
        _TextRender->SetText(FText::FromString(Text));
    }
    else {
        _TextRender->SetText(Text.IsEmpty() ? _PrevText : FText::FromString(Text));
        SetRelativeRotation(FRotator(0, 0, 0));
    }
}

/*********************************************** DELEGATES ***************************************/
void UInputMenu::AddOnInputMenuDelegate() {
    if(_InputMenuPressedDelegate.IsBound())
        _OnInputMenuPressedDelegateHandle = _InputMenuPressedEvent.Add(_InputMenuPressedDelegate);
    
    if (_InputMenuReleasedDelegate.IsBound())
        _OnInputMenuReleasedDelegateHandle = _InputMenuReleasedEvent.Add(_InputMenuReleasedDelegate);
}

void UInputMenu::ClearOnInputMenuDelegate() {
    _InputMenuPressedEvent.Remove(_OnInputMenuPressedDelegateHandle);
    _InputMenuReleasedEvent.Remove(_OnInputMenuReleasedDelegateHandle);
}