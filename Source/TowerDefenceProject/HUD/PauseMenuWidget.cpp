
#include "HUD/PauseMenuWidget.h"
#include "Components/Button.h"
#include "Player/TopDownPawn.h"
#include "Kismet/GameplayStatics.h"


void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// had to remove destruct and add remove dynamic here, as i never destroy the widget...
	if (ResumeButton)
	{
		ResumeButton->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnResumeClicked); // got duplicate dynamic shite stuff, so added this
		ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnRestartClicked);
		RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClicked);
	}

	if (ExitToMenuButton)
	{
		ExitToMenuButton->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnExitToMenuClicked);
		ExitToMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnExitToMenuClicked);
	}

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnQuitGameClicked);
		QuitGameButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitGameClicked);
	}
}

void UPauseMenuWidget::OnResumeClicked()
{
	UE_LOG(LogTemp, Log, TEXT("PM_Widget: Resume Clicked"));

	// enter player pawn and run function
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ATopDownPawn* Pawn = Cast<ATopDownPawn>(PC->GetPawn()))
		{
			Pawn->TogglePause();
		}
	}
}

void UPauseMenuWidget::OnRestartClicked()
{
	UE_LOG(LogTemp, Log, TEXT("PM_Widget: Restart Clicked"));

	if (!(CurrentLevel == NAME_None))
	{
		UGameplayStatics::OpenLevel(this, CurrentLevel);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PM_Widget: No CurrentLevel name given!"));
	}
}

void UPauseMenuWidget::OnExitToMenuClicked()
{
	UE_LOG(LogTemp, Log, TEXT("PM_Widget: ExitToMenu Clicked"));
	
	if (!(MainMenuLevel == NAME_None))
	{
		UGameplayStatics::OpenLevel(this, MainMenuLevel);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PM_Widget: No MainMenuLevel name given!"));
	}
}

void UPauseMenuWidget::OnQuitGameClicked()
{
	UE_LOG(LogTemp, Log, TEXT("PM_Widget: QuitGame Clicked"));

	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}
