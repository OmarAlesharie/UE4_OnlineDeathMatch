// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeathMatchGameMode.h"
#include "DeathMatchCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADeathMatchGameMode::ADeathMatchGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
