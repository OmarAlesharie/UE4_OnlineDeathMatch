// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameStateBase.h"

#include "Net/UnrealNetwork.h"

void ADeathMatchGameStateBase::SetGameplayState(EGameplayState NewState)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        EGameplayState OldState = GameplayState;

        GameplayState = NewState;
    }
}

EGameplayState ADeathMatchGameStateBase::GetGameplayState()
{
    return GameplayState;
}

void ADeathMatchGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADeathMatchGameStateBase, GameplayState);
}
