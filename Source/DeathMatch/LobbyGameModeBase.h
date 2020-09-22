// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/GameModeBase.h"
#include "LobbyGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCH_API ALobbyGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:

    ALobbyGameModeBase();
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
                          FString& ErrorMessage) override;

    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual void Logout(AController* Exiting) override;

    UFUNCTION(Exec)
    void GetCurrentPlayersCount();

private:

    void StartGame();

    FTimerHandle GameStartTimer;

    FTimerHandle DisplayPlayersCountTimer;
};
