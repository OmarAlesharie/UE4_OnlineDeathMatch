// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MasterMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCH_API UMasterMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup();
	void Teardown();
	
};
