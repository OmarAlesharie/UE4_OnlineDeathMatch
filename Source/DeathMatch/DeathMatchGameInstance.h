// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/GameInstance.h"
#include "DeathMatchGameInstance.generated.h"


class UMainMenuUserWidget;
/**
 * 
 */
UCLASS()
class DEATHMATCH_API UDeathMatchGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDeathMatchGameInstance(const FObjectInitializer & ObjectInitializer);

	virtual void Init();

	UFUNCTION(Exec)
    void Host(FString ServerName);

	UFUNCTION(Exec)
    void Join(uint32 Index);

	void StartSession();
	
	void LoadMainMenu();

	void RefreshServerList();

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	TSubclassOf<class UUserWidget> MenuClass;

	UMainMenuUserWidget* Menu;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	FString DesiredServerName;
	void CreateSession();

	void OnNetworkFailure(UWorld* world, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
};
