// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "NWGameInstance.h"

#include "PlayerControllerLobby.h"
#include "Menu3D.h"
#include "InputMenu.h"

/* VR Includes */
#include "HeadMountedDisplay.h"


void UNWGameInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UNWGameInstance, _MaxPlayers);
    DOREPLIFETIME(UNWGameInstance, _ServerName);
}

UNWGameInstance::UNWGameInstance(const FObjectInitializer& OI) : Super(OI) {
    /** Bind function for CREATING a Session */
    OnCreateSessionCompleteDelegate =
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnCreateSessionComplete);
    OnStartSessionCompleteDelegate =
        FOnStartSessionCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnStartOnlineGameComplete);

    /** Bind function for FINDING a Session */
    OnFindSessionsCompleteDelegate =
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnFindSessionsComplete);

    /** Bind function for JOINING a Session */
    OnJoinSessionCompleteDelegate =
        FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnJoinSessionComplete);
    
    /** Bind function for DESTROYING a Session */
    OnDestroySessionCompleteDelegate =
        FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNWGameInstance::OnDestroySessionComplete);

    _MapLobbyName = USettings::Get()->LevelLobby.GetAssetName();
    
    _MaxPlayers = 2;
    _ServerName = "";
    _SessionOwner = "";
    _IsVRMode = false;

    static ConstructorHelpers::FClassFinder<AActor> BoyClassFinder(TEXT(
        "/Game/BluePrints/Characters/FPCharacterBoy"));
    _BoyClass = BoyClassFinder.Class;
    static ConstructorHelpers::FClassFinder<AActor> GirlClassFinder(TEXT(
        "/Game/BluePrints/Characters/FPCharacterGirl"));
    _GirlClass = GirlClassFinder.Class;

    static ConstructorHelpers::FClassFinder<AActor> VRBoyClassFinder(TEXT(
        "/Game/BluePrints/Characters/VRCharacterBoy"));
    _VRBoyClass = VRBoyClassFinder.Class;
    static ConstructorHelpers::FClassFinder<AActor> VRGirlClassFinder(TEXT(
        "/Game/BluePrints/Characters/VRCharacterGirl"));
    _VRGirlClass = VRGirlClassFinder.Class;

    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT(
        "/Game/BluePrints/Characters/FPCharacter_BP"));
    _DefaultCharacterClass = PlayerPawnClassFinder.Class;

    static ConstructorHelpers::FClassFinder<APawn> PlayerVRPawnClassFinder(TEXT(
        "/Game/BluePrints/Characters/VRCharacter_BP"));
    _VRDefaultCharacterClass = PlayerVRPawnClassFinder.Class;

    _MenuOptions.bComfortMode = false;
}

IOnlineSessionPtr UNWGameInstance::GetSessions() {
    IOnlineSessionPtr Sessions;
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub) {
        Sessions = OnlineSub->GetSessionInterface();
    }
    else {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
    }
    return Sessions;
}

void UNWGameInstance::InitGame() {
    /* SWITCH PLAYER MODE */
    if (FParse::Param(FCommandLine::Get(), TEXT("vr"))) _IsVRMode = true;

    if(GEngine) {
        IHeadMountedDisplay* HMD = (IHeadMountedDisplay*)(GEngine->HMDDevice.Get());
        if (HMD) {
            HMD->EnableHMD(_IsVRMode);
            HMD->EnableStereo(_IsVRMode);
        }
    }
    ULibraryUtils::Log(FString::Printf(TEXT("_IsVRMode: %s"), _IsVRMode ? TEXT("true") : TEXT("false")));


    APlayerControllerLobby* const PlayerControllerLobby = Cast<APlayerControllerLobby>(
                                                                GetFirstLocalPlayerController());
    AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    if (PlayerControllerLobby && GameMode) {
        TSubclassOf<ACharacter> CharacterClass = _IsVRMode ? _VRDefaultCharacterClass :
                                                             _DefaultCharacterClass;
        FTransform Transform = GameMode->FindPlayerStart(PlayerControllerLobby, TEXT("playerstart"))
                                            ->GetActorTransform();
        APawn* Actor = Cast<APawn>(GetWorld()->SpawnActor(CharacterClass, &Transform));
        if (Actor) {
            PlayerControllerLobby->Possess(Actor);
        }

        PlayerControllerLobby->CLIENT_CreateMenu();
    }
}

/**************************************** BLUEPRINTS *********************************************/
void UNWGameInstance::LaunchLobby() {
    _PlayerInfoSaved.Name = "host";
    _PlayerInfoSaved.CharacterClass = _IsVRMode ? _VRBoyClass : _BoyClass;
    _PlayerInfoSaved.IsHost = true;

    DestroySession();

    _ServerName = "ServerName";
    ULocalPlayer* const Player = GetFirstGamePlayer();
    HostSession(Player->GetPreferredUniqueNetId(), GameSessionName, true, true, _MaxPlayers);
}

void UNWGameInstance::FindOnlineGames() {
    ULocalPlayer* const Player = GetFirstGamePlayer();
    FindSessions(Player->GetPreferredUniqueNetId(), true, true);
}

void UNWGameInstance::JoinOnlineGame() {
    _PlayerInfoSaved.Name = "guest";
    _PlayerInfoSaved.CharacterClass = _IsVRMode ? _VRGirlClass : _GirlClass;
    _PlayerInfoSaved.IsHost = false;

    ULocalPlayer* const Player = GetFirstGamePlayer();
    FOnlineSessionSearchResult SearchResult;
    if (_SessionSearch->SearchResults.Num() > 0) {
        for (int32 i = 0; i < _SessionSearch->SearchResults.Num(); i++) {
            SearchResult = _SessionSearch->SearchResults[i];
            if (SearchResult.Session.OwningUserId != Player->GetPreferredUniqueNetId()) {
                JoinAtSession(Player->GetPreferredUniqueNetId(), GameSessionName, SearchResult);
                break;
            }
        }
    }
}

void UNWGameInstance::DestroySession() {
    IOnlineSessionPtr Sessions = GetSessions();
    if (Sessions.IsValid()) {
        Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
        Sessions->DestroySession(GameSessionName);
    }
}

/**************************************** SESSION ************************************************/
bool UNWGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName,
                                  bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers) {
    IOnlineSessionPtr Sessions = GetSessions();
    if (Sessions.IsValid() && UserId.IsValid()) {
        _SessionSettings = MakeShareable(new FOnlineSessionSettings());

        _SessionSettings->bIsLANMatch = bIsLAN;
        _SessionSettings->bUsesPresence = bIsPresence;
        _SessionSettings->NumPublicConnections = MaxNumPlayers;
        _SessionSettings->NumPrivateConnections = 0;
        _SessionSettings->bAllowInvites = true;
        _SessionSettings->bAllowJoinInProgress = true;
        _SessionSettings->bShouldAdvertise = true;
        _SessionSettings->bAllowJoinViaPresence = true;
        _SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
        _SessionSettings->Set(SETTING_MAPNAME, _MapLobbyName,
                              EOnlineDataAdvertisementType::ViaOnlineService);
        OnCreateSessionCompleteDelegateHandle = 
            Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

        return Sessions->CreateSession(*UserId, SessionName, *_SessionSettings);
    }
    return false;
}

void UNWGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) {
    IOnlineSessionPtr Sessions = GetSessions();
    if (Sessions.IsValid()) {
        Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
        OnStartSessionCompleteDelegateHandle =
            Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
        Sessions->StartSession(GameSessionName);
    }
}

void UNWGameInstance::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful) {
    IOnlineSessionPtr Sessions = GetSessions();
    if (Sessions.IsValid()) {
        Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
    }
    if (bWasSuccessful) {
        UGameplayStatics::OpenLevel(GetWorld(), FName(*_MapLobbyName), true, "listen");
    }
}

void UNWGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence) {
    IOnlineSessionPtr Sessions = GetSessions();
    if (Sessions.IsValid() && UserId.IsValid()) {
        _SessionSearch = MakeShareable(new FOnlineSessionSearch());
        _SessionSearch->bIsLanQuery = bIsLAN;
        _SessionSearch->MaxSearchResults = 20;
        _SessionSearch->PingBucketSize = 50;
        _SessionSearch->TimeoutInSeconds = 15;

        //if (bIsPresence) { 
        //    _SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
        //}

        TSharedRef<FOnlineSessionSearch> SearchSettingsRef = _SessionSearch.ToSharedRef();
        OnFindSessionsCompleteDelegateHandle = 
            Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
        _SessionOwner = "";
        Sessions->FindSessions(*UserId, SearchSettingsRef);
    }
}

void UNWGameInstance::OnFindSessionsComplete(bool bWasSuccessful) {
    ULibraryUtils::Log(FString::Printf(TEXT("Number of Sessions found: %d"),
                                       _SessionSearch->SearchResults.Num()), 3, 15);
    IOnlineSessionPtr Sessions = GetSessions();
    FString Result = "";
    if (Sessions.IsValid()) {
        Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
        for (int32 i = 0; i < _SessionSearch->SearchResults.Num(); i++) {
            if (_SessionSearch->SearchResults[i].Session.NumOpenPublicConnections > 0) {
                Result = _SessionSearch->SearchResults[i].Session.OwningUserName;
            }
        }
    }
    else Result = "NO VALID SESSIONS";

    _SessionOwner = Result.Len() > 0 ? Result : "NO SESSIONS FOUND";
}

bool UNWGameInstance::JoinAtSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName,
                                    const FOnlineSessionSearchResult& SearchResult) {
    bool bSuccessful = false;
    IOnlineSessionPtr Sessions = GetSessions();
    if (Sessions.IsValid() && UserId.IsValid()) {
        OnJoinSessionCompleteDelegateHandle = 
            Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
        bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
    }
    return bSuccessful;
}

void UNWGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {
    IOnlineSessionPtr Sessions = GetSessions();
    if (Sessions.IsValid()) {
        Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
        APlayerController * const PlayerController = GetFirstLocalPlayerController();
        FString TravelURL;
        if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL)) {
            PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
        }
    }
}

void UNWGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful) {
    IOnlineSessionPtr Sessions = GetSessions();
    if (Sessions.IsValid()) {
        Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
        if (bWasSuccessful) {
            //UGameplayStatics::OpenLevel(GetWorld(), _MapMenuName, true);
            APlayerController * const PlayerController = GetFirstLocalPlayerController();
            if (PlayerController) PlayerController->ClientReturnToMainMenu("");
        }
    }
}

/*********************************** MENU INTERFACE **********************************************/
AMenu3D* UNWGameInstance::CreateMenuMain() {
    _MenuActor = GetWorld()->SpawnActor<AMenu3D>();

    /*** (0)MAIN MENU ***/
    UMenuPanel* MenuMain = NewObject<UMenuPanel>(_MenuActor, FName("MenuMain"));
    UInputMenu* Slot_NewGame = NewObject<UInputMenu>(_MenuActor, FName("NEW GAME"));
    Slot_NewGame->_NavigateMenuIndex = 1;
    UInputMenu* Slot_Options = NewObject<UInputMenu>(_MenuActor, FName("OPTIONS"));
    Slot_Options->_NavigateMenuIndex = 2;
    UInputMenu* Slot_ExitGame = NewObject<UInputMenu>(_MenuActor, FName("EXIT GAME"));
    Slot_ExitGame->_InputMenuReleasedDelegate.BindUObject(this, &UNWGameInstance::OnExitGame);
    Slot_ExitGame->AddOnInputMenuDelegate();

    _MenuActor->AddSubmenu(MenuMain);
    MenuMain->AddMenuInput(Slot_NewGame);
    MenuMain->AddMenuInput(Slot_Options);
    MenuMain->AddMenuInput(Slot_ExitGame);

    ///*** (1)NEW GAME MENU ***/
    //UMenuPanel* MenuNewGame = NewObject<UMenuPanel>(_MenuActor, FName("MenuNewGame"));
    //UInputMenu* Slot_HostGame = NewObject<UInputMenu>(_MenuActor, FName("Slot_HostGame"));
    //Slot_HostGame->_InputMenuReleasedDelegate.BindUObject(this, &UNWGameInstance::LaunchLobby);
    //Slot_HostGame->AddOnInputMenuDelegate();
    //UInputMenu* Slot_FindGame = NewObject<UInputMenu>(_MenuActor, FName("Slot_FindGame"));
    //Slot_FindGame->_NavigateMenuIndex = 3;
    //Slot_FindGame->_InputMenuReleasedDelegate.BindUObject(this, &UNWGameInstance::FindOnlineGames);
    //Slot_FindGame->AddOnInputMenuDelegate();

    //_MenuActor->AddSubmenu(MenuNewGame);
    //MenuNewGame->AddMenuInput(Slot_HostGame);
    //MenuNewGame->AddMenuInput(Slot_FindGame);

    ///*** (2)OPTIONS MENU ***/
    //UMenuPanel* MenuOptions = NewObject<UMenuPanel>(_MenuActor, FName("MenuOptions"));
    //UInputMenu* Slot_ComfortMode = NewObject<UInputMenu>(_MenuActor, FName("Slot_ComfortMode"));
    //Slot_ComfortMode->_InputMenuReleasedDelegate.BindUObject(this, &UNWGameInstance::SwitchComfortMode);
    //Slot_ComfortMode->AddOnInputMenuDelegate();

    //_MenuActor->AddSubmenu(MenuOptions);
    //MenuOptions->AddMenuInput(Slot_ComfortMode);

    ///*** (3)FIND GAME MENU ***/
    //UMenuPanel* MenuFindGame = NewObject<UMenuPanel>(_MenuActor, FName("MenuFindGame"));
    //UInputMenu* Slot_JoinGame = NewObject<UInputMenu>(_MenuActor, FName("Slot_JoinGame"));
    //Slot_JoinGame->_InputMenuReleasedDelegate.BindUObject(this, &UNWGameInstance::JoinOnlineGame);
    //Slot_JoinGame->AddOnInputMenuDelegate();

    //_MenuActor->AddSubmenu(MenuFindGame);
    //MenuFindGame->AddMenuInput(Slot_JoinGame);

    ULibraryUtils::Log("MENU CREATED");

    return _MenuActor;
}

AMenu3D* UNWGameInstance::CreateMenuLobby() {
    _MenuActor = NewObject<AMenu3D>(_MenuActor, FName("MENU"));

    return _MenuActor;
}

AMenu3D* UNWGameInstance::CreateMenuPlay() {
    _MenuActor = NewObject<AMenu3D>(_MenuActor, FName("MENU"));

    return _MenuActor;
}

/*********************************** BINDINGS ****************************************************/
void UNWGameInstance::OnExitGame() {
    FGenericPlatformMisc::RequestExit(false);
}

void UNWGameInstance::SwitchComfortMode() {
    _MenuOptions.bComfortMode = !_MenuOptions.bComfortMode;
}