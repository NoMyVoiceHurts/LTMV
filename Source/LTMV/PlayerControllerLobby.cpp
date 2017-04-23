// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "PlayerControllerLobby.h"

#include "GameModeLobby.h"
#include "PlayerCharacter.h"


APlayerControllerLobby::APlayerControllerLobby(const FObjectInitializer& OI) : Super(OI) {
    /* MENU */
    _IsMenuHidden = true;
    static ConstructorHelpers::FClassFinder<AActor> MenuClassFinder(TEXT(
        "/Game/BluePrints/HUD/MenuLobbyActor_BP"));
    _MenuClass = MenuClassFinder.Class;

    GConfig->GetString(
        TEXT("/Script/EngineSettings.GameMapsSettings"),
        TEXT("GameDefaultMap"),
        _MapMainMenu,
        GEngineIni
    );
}

void APlayerControllerLobby::SetupInputComponent() {
    Super::SetupInputComponent();
    InputComponent->BindAction("Menu", IE_Released, this, &APlayerControllerLobby::ToogleMenu);

    /* USE ITEM */
    InputComponent->BindAction("ClickLeft", IE_Pressed, this, &APlayerControllerLobby::UseLeftPressed);
    InputComponent->BindAction("ClickLeft", IE_Released, this, &APlayerControllerLobby::UseLeftReleased);
}

void APlayerControllerLobby::CLIENT_InitialSetup_Implementation() {
    UNWGameInstance* gameInstance = Cast<UNWGameInstance>(GetGameInstance());
    if (gameInstance) SERVER_CallUpdate(gameInstance->_PlayerInfoSaved, false);
}

bool APlayerControllerLobby::SERVER_CallUpdate_Validate(FPlayerInfo info,
                                                        bool changedStatus) { return true; }
void APlayerControllerLobby::SERVER_CallUpdate_Implementation(FPlayerInfo info,
                                                              bool changedStatus) {
    AGameModeLobby* gameMode = Cast<AGameModeLobby>(GetWorld()->GetAuthGameMode());
    if (gameMode) gameMode->SERVER_SwapCharacter(this, info, changedStatus);
}

void APlayerControllerLobby::CLIENT_CreateMenu_Implementation(TSubclassOf<AActor> menuClass) {
    APawn* pawn = GetPawn();
    if (pawn) {
        UCameraComponent* cameraComp = Cast<UCameraComponent>(pawn->FindComponentByClass<UCameraComponent>());
        if (cameraComp) {
            FVector position = cameraComp->GetComponentLocation();
            FRotator rotation = cameraComp->GetComponentRotation();

            // Save a Reference of the spawned actor??
            GetWorld()->SpawnActor(menuClass, &position, &rotation);
        }
    }
}

void APlayerControllerLobby::AfterPossessed() {
    /* CLIENT-SERVER EXCEPTION  */
    if (!_ClientPossesed) {
        APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
        if (PlayerCharacter) {
            PlayerCharacter->AfterPossessed(true);
            _ClientPossesed = true;
        }
    }
}

void APlayerControllerLobby::OnRep_Pawn() {
    Super::OnRep_Pawn();
    /* CLIENT-SERVER EXCEPTION  */
    AfterPossessed();
}

/****************************************** ACTION MAPPINGS **************************************/
/******** USE ITEM LEFT *********/
void APlayerControllerLobby::UseLeftPressed() {
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
    if (PlayerCharacter) {
        PlayerCharacter->FindComponentByClass<UWidgetInteractionComponent>()->
            PressPointerKey(EKeys::LeftMouseButton);
    }
}

void APlayerControllerLobby::UseLeftReleased() {
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
    if (PlayerCharacter) {
        PlayerCharacter->FindComponentByClass<UWidgetInteractionComponent>()->
            ReleasePointerKey(EKeys::LeftMouseButton);
    }
}

/*************** TRIGGER MENU *************/
void APlayerControllerLobby::ToogleMenu() {
    if (!_MapMainMenu.Contains(GetWorld()->GetMapName())) {
        APawn* pawn = GetPawn();
        if (pawn) {
            if (_IsMenuHidden) {
                UCameraComponent* cameraComp = Cast<UCameraComponent>(pawn->FindComponentByClass<UCameraComponent>());
                if (cameraComp) {
                    FVector position = cameraComp->GetComponentLocation();
                    FRotator rotation = cameraComp->GetComponentRotation();

                    if (_MenuActor) {
                        ULibraryUtils::SetActorEnable(_MenuActor);
                        _MenuActor->SetActorLocationAndRotation(position,
                                                                rotation,
                                                                false,
                                                                nullptr,
                                                                ETeleportType::TeleportPhysics);
                    }
                    else {
                        _MenuActor = GetWorld()->SpawnActor(_MenuClass, &position, &rotation);
                    }
                }
            }
            else {
                ULibraryUtils::SetActorEnable(_MenuActor, false);
            }
            _IsMenuHidden = !_IsMenuHidden;
        }
    }
}

/***************** EXIT GAME **************/
void APlayerControllerLobby::ExitGame() {
    FGenericPlatformMisc::RequestExit(false);
}