// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DeathMatchGameStateBase.generated.h"


UENUM(BlueprintType)
enum class EGameplayState : uint8
{
	WAitingToStart,
	GameStarted,
	GameEnds,
};

/**
 * 
 */

UCLASS()
class DEATHMATCH_API ADeathMatchGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(Replicated,BlueprintReadOnly, Category="GameState")
	EGameplayState GameplayState;
	
public:

	void SetGameplayState(EGameplayState NewState);
	EGameplayState GetGameplayState();
	
};
