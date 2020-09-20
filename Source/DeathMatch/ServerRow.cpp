// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRow.h"

#include "Components/Button.h"
#include "MainMenuUserWidget.h"

void UServerRow::Setup(UMainMenuUserWidget* InParent, uint32 InIndex)
{
    Parent = InParent;
    Index = InIndex;
    RowButton->OnClicked.AddDynamic(this, &UServerRow::OnClicked);
}

void UServerRow::OnClicked()
{
    
}
