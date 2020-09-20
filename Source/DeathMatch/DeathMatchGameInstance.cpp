// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "MainMenuUserWidget.h"

const static FName SESSION_NAME = TEXT("GameSession");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

UDeathMatchGameInstance::UDeathMatchGameInstance(const FObjectInitializer& ObjectInitializer)
{
    ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/UI/WBP_MainMenu"));
    if (!ensure(MenuBPClass.Class != nullptr)) return;

    MenuClass = MenuBPClass.Class;
}

void UDeathMatchGameInstance::Init()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *Subsystem->GetSubsystemName().ToString());
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
                this, &UDeathMatchGameInstance::OnCreateSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
                this, &UDeathMatchGameInstance::OnDestroySessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
                this, &UDeathMatchGameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
                this, &UDeathMatchGameInstance::OnJoinSessionComplete);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Found no subsystem"));
    }

    // Bined to network Failure Events
    GetEngine()->OnNetworkFailure().AddUObject(this, &UDeathMatchGameInstance::OnNetworkFailure);
}

void UDeathMatchGameInstance::Host(FString ServerName)
{
    DesiredServerName = ServerName;
    if (SessionInterface.IsValid())
    {
        auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
        if (ExistingSession != nullptr)
        {
            SessionInterface->DestroySession(SESSION_NAME);
        }
        else
        {
            CreateSession();
        }
    }
}

void UDeathMatchGameInstance::Join(uint32 Index)
{
    if (!SessionInterface.IsValid()) return;
    if (!SessionSearch.IsValid()) return;

    if (Menu != nullptr)
    {
        Menu->Teardown();
    }

    SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UDeathMatchGameInstance::StartSession()
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->StartSession(SESSION_NAME);
    }
}

void UDeathMatchGameInstance::LoadMainMenu()
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel("/Game/MenuSystem/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UDeathMatchGameInstance::RefreshServerList()
{
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    if (SessionSearch.IsValid())
    {
        //SessionSearch->bIsLanQuery = true;
        SessionSearch->MaxSearchResults = 100;
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
        UE_LOG(LogTemp, Warning, TEXT("Starting Find Session"));
        SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
    }
}

void UDeathMatchGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
    if (!Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not create session"));
        return;
    }

    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;

    World->ServerTravel("/Game/PuzzlePlatforms/Maps/Lobby?listen");
}

void UDeathMatchGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
    if (Success)
    {
        CreateSession();
    }
}

void UDeathMatchGameInstance::OnFindSessionsComplete(bool Success)
{
    if (Success && SessionSearch.IsValid() && Menu != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Finished Find Session"));

        TArray<FServerData> ServerNames;
        for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
        {
            UE_LOG(LogTemp, Warning, TEXT("Found session names: %s"), *SearchResult.GetSessionIdStr());
            FServerData Data;
            Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
            Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
            Data.HostUsername = SearchResult.Session.OwningUserName;
            FString ServerName;
            if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
            {
                Data.Name = ServerName;
            }
            else
            {
                Data.Name = "Could not find name.";
            }
            ServerNames.Add(Data);
        }

        Menu->SetServerList(ServerNames);
    }
}

void UDeathMatchGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!SessionInterface.IsValid()) return;

    FString Address;
    if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not get connect string."));
        return;
    }

    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UDeathMatchGameInstance::CreateSession()
{
    if (SessionInterface.IsValid())
    {
        FOnlineSessionSettings SessionSettings;
        if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
        {
            SessionSettings.bIsLANMatch = true;
        }
        else
        {
            SessionSettings.bIsLANMatch = false;
        }
        SessionSettings.NumPublicConnections = 3;
        SessionSettings.bShouldAdvertise = true;
        SessionSettings.bUsesPresence = true;
        SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName,
                            EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
    }
}

void UDeathMatchGameInstance::OnNetworkFailure(UWorld* world, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
                                               const FString& ErrorString)
{
    LoadMainMenu();

    switch (FailureType)
    {
    case ENetworkFailure::NetDriverAlreadyExists: break;
    case ENetworkFailure::NetDriverCreateFailure: break;
    case ENetworkFailure::NetDriverListenFailure: break;
    case ENetworkFailure::ConnectionLost: break;
    case ENetworkFailure::ConnectionTimeout: break;
    case ENetworkFailure::FailureReceived: break;
    case ENetworkFailure::OutdatedClient: break;
    case ENetworkFailure::OutdatedServer: break;
    case ENetworkFailure::PendingConnectionFailure: break;
    case ENetworkFailure::NetGuidMismatch: break;
    case ENetworkFailure::NetChecksumMismatch: break;
    default: ;
    }
}
