
#include "HUD/MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "MainMenuWidget.h"

void UMainMenuWidget::Setup()
{
	// Ensure widget is added to viewport
	if (!IsInViewport())
	{
		AddToViewport();
	}

	// Set inputmode to UI-only
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}

	// Default level if not set
	if (LevelToLoad == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: LevelToLoad not set!"));
	}
}

void UMainMenuWidget::NativeDestruct()
{
	if (StartButton)
	{
		// if it does not work, change removeDynamic -> RemoveAll
		StartButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnStartClicked);
	}
	if (QuitButton)
	{
		// if it does not work, change removeDynamic -> RemoveAll
		QuitButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}

	Super::NativeDestruct();
}

void UMainMenuWidget::OnStartClicked()
{
	if (LevelToLoad != NAME_None)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenu: OpenLevel: %s"), *LevelToLoad.ToString());
		UGameplayStatics::OpenLevel(GetWorld(), LevelToLoad);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: No level specified!"));
	}
}

void UMainMenuWidget::OnQuitClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenu: Quit"));
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}


