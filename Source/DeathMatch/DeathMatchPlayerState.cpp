// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerState.h"

#include "DeathMatchGameMode.h"
#include "Net/UnrealNetwork.h"


ADeathMatchPlayerState::ADeathMatchPlayerState(const FObjectInitializer& ObjectInitializer)
{
    Lives = 3;
}

void ADeathMatchPlayerState::RemoveOneLife()
{
    if (GetLocalRole() == ROLE_Authority)
    {
        if (Lives > 0)
        {
            Lives--;
        }
    }
}

int32 ADeathMatchPlayerState::GetCurrentLives()
{
    return Lives;
}

void ADeathMatchPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADeathMatchPlayerState, Lives);
}
