// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameModeBase.h"

#include "DeathMatchCharacter.h"
#include "TimerManager.h"
#include "DeathMatchGameInstance.h"
#include "DeathMatchGameStateBase.h"

ALobbyGameModeBase::ALobbyGameModeBase()
{
    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    GameStateClass = ADeathMatchGameStateBase::StaticClass();
}

void ALobbyGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
                                  FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    
    GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, TEXT("PreLogin()"));
    if (GetNumPlayers() >= 3)
    {
        ErrorMessage = "Server is full";
    }
}

void ALobbyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    auto GameInstance = Cast<UDeathMatchGameInstance>(GetGameInstance());

    if (GameInstance == nullptr)
    {
        return;
    }
    
    if (GetNumPlayers() >= GameInstance->GetMaxNumOfPlayrs())
    {
        GetWorldTimerManager().SetTimer(GameStartTimer, this, &ALobbyGameModeBase::StartGame, 10);
    }

    GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Magenta,
        FString::Printf(TEXT("We have a new player %d player(s) in the server"), GetNumPlayers()));
}

void ALobbyGameModeBase::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    GetWorldTimerManager().SetTimer(DisplayPlayersCountTimer,this ,&ALobbyGameModeBase::GetCurrentPlayersCount,1);
    GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Magenta,TEXT("Some one leaves the Server"));
}

void ALobbyGameModeBase::GetCurrentPlayersCount()
{
    GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange,
        FString::Printf(TEXT(" %d player(s) in the server"), GetNumPlayers()));
}

void ALobbyGameModeBase::StartGame()
{
    auto GameInstance = Cast<UDeathMatchGameInstance>(GetGameInstance());

    if (GameInstance == nullptr) return;

    GameInstance->StartSession();

    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;

    bUseSeamlessTravel = true;

    ADeathMatchGameStateBase* GS = GetGameState<ADeathMatchGameStateBase>();
    if (GS)
    {
        GS->SetGameplayState(EGameplayState::GameStarted);
    }
	
    if (World->ServerTravel("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap",true))
    {
        GEngine->AddOnScreenDebugMessage(-1,5, FColor::Green, TEXT("ServerTravel() Succeeded"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1,5, FColor::Red, TEXT("ServerTravel() Failed"));
    }
}
