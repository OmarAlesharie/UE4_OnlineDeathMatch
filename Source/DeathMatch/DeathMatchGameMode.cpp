// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeathMatchGameMode.h"
#include "DeathMatchCharacter.h"
#include "DeathMatchGameStateBase.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

ADeathMatchGameMode::ADeathMatchGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	GameStateClass = ADeathMatchGameStateBase::StaticClass();
}

void ADeathMatchGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void ADeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void ADeathMatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void ADeathMatchGameMode::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle Timer;
	GetWorldTimerManager().SetTimer(Timer, this,&ADeathMatchGameMode::SetFireOn,3);
}

void ADeathMatchGameMode::SetFireOn()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		ADeathMatchCharacter* Character = Cast<ADeathMatchCharacter>(PC->GetCharacter());

		if (Character)
		{
			Character->bCanFire = true;
		}
	}
}
