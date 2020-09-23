// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DeathMatchGameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "UObject/ObjectMacros.h"

#include "DeathMatchPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCH_API ADeathMatchPlayerState : public APlayerState
{
	GENERATED_BODY()

	UPROPERTY(Replicated)
	int32 Lives;

	UPROPERTY(Replicated)
	int32 Kills;
	
public:

	ADeathMatchPlayerState(const FObjectInitializer & ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category="MyGameState")
	void RemoveOneLife();

	UFUNCTION(BlueprintCallable, Category="MyGameState")
	int32 GetCurrentLives();	
};
