// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuUserWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/WidgetSwitcher.h"
#include "DeathMatchGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "ServerRow.h"
#include "Components/TextBlock.h"

UMainMenuUserWidget::UMainMenuUserWidget(const FObjectInitializer& ObjectInitializer)
{
    ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/Game/MenuSystem/WBP_ServerRow"));
    if (!ensure(ServerRowBPClass.Class != nullptr)) return;

    ServerRowClass = ServerRowBPClass.Class;
}

void UMainMenuUserWidget::SetServerList(TArray<FServerData> ServerNames)
{
    UWorld* World = this->GetWorld();
    if (!ensure(World != nullptr)) return;

    ServerList->ClearChildren();

    uint32 i = 0;
    for (const FServerData& ServerData : ServerNames)
    {
        UServerRow* Row = CreateWidget<UServerRow>(World, ServerRowClass);
        if (!ensure(Row != nullptr)) return;

        Row->ServerName->SetText(FText::FromString(ServerData.Name));
        Row->HostUser->SetText(FText::FromString(ServerData.HostUsername));
        FString FractionText = FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers);
        Row->ConnectionFraction->SetText(FText::FromString(FractionText));
        Row->Setup(this, i);
        ++i;

        ServerList->AddChild(Row);
    }
}

void UMainMenuUserWidget::SelectIndex(uint32 Index)
{
    SelectedIndex = Index;
    UpdateChildren();
}

bool UMainMenuUserWidget::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;
	
    if (!ensure(HostButton != nullptr)) return false;
    HostButton->OnClicked.AddDynamic(this, &UMainMenuUserWidget::OpenHostMenu);

    if (!ensure(CancelHostMenuButton != nullptr)) return false;
    CancelHostMenuButton->OnClicked.AddDynamic(this, &UMainMenuUserWidget::OpenMainMenu);

    if (!ensure(ConfirmHostMenuButton != nullptr)) return false;
    ConfirmHostMenuButton->OnClicked.AddDynamic(this, &UMainMenuUserWidget::HostServer);

    if (!ensure(JoinButton != nullptr)) return false;
    JoinButton->OnClicked.AddDynamic(this, &UMainMenuUserWidget::OpenJoinMenu);

    if (!ensure(QuitButton != nullptr)) return false;
    QuitButton->OnClicked.AddDynamic(this, &UMainMenuUserWidget::QuitPressed);

    if (!ensure(CancelJoinMenuButton != nullptr)) return false;
    CancelJoinMenuButton->OnClicked.AddDynamic(this, &UMainMenuUserWidget::OpenMainMenu);

    if (!ensure(ConfirmJoinMenuButton != nullptr)) return false;
    ConfirmJoinMenuButton->OnClicked.AddDynamic(this, &UMainMenuUserWidget::JoinServer);

    return true;
}

void UMainMenuUserWidget::HostServer()
{
    FString ServerName = ServerHostName->Text.ToString();

    UDeathMatchGameInstance* GI = Cast<UDeathMatchGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI)
    {
        GI->Host(ServerName);
    }
}

void UMainMenuUserWidget::JoinServer()
{
    UDeathMatchGameInstance* GI = Cast<UDeathMatchGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (SelectedIndex.IsSet() && GI)
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected index %d."), SelectedIndex.GetValue());
        GI->Join(SelectedIndex.GetValue());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected index not set."));
    }
}

void UMainMenuUserWidget::OpenHostMenu()
{
    MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenuUserWidget::OpenJoinMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(JoinMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(JoinMenu);

    UDeathMatchGameInstance* GI = Cast<UDeathMatchGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI) {
        GI->RefreshServerList();
    }
}

void UMainMenuUserWidget::OpenMainMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(JoinMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenuUserWidget::QuitPressed()
{
    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ConsoleCommand("quit");
}

void UMainMenuUserWidget::UpdateChildren()
{
    for (int32 i = 0; i < ServerList->GetChildrenCount(); ++i)
    {
        auto Row = Cast<UServerRow>(ServerList->GetChildAt(i));
        if (Row != nullptr)
        {
            Row->Selected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
        }
    }
}
