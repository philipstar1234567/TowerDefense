
#include "HUD/EndGameWidget.h"
#include "Components/Button.h"
#include "Player/TopDownPawn.h"
#include "Kismet/GameplayStatics.h"

void UEndGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RestartButton)
	{
		RestartButton->OnClicked.RemoveDynamic(this, &UEndGameWidget::OnRestartClicked);
		RestartButton->OnClicked.AddDynamic(this, &UEndGameWidget::OnRestartClicked);
	}

	if (ExitToMenuButton)
	{
		ExitToMenuButton->OnClicked.RemoveDynamic(this, &UEndGameWidget::OnExitToMenuClicked);
		ExitToMenuButton->OnClicked.AddDynamic(this, &UEndGameWidget::OnExitToMenuClicked);
	}

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.RemoveDynamic(this, &UEndGameWidget::OnQuitGameClicked);
		QuitGameButton->OnClicked.AddDynamic(this, &UEndGameWidget::OnQuitGameClicked);
	}

	if (Headline)
	{
		FText EndResult = bWinning ? FText::FromString("Win") : FText::FromString("Loss");

		Headline->SetText(EndResult);
	}

	// pause the game, since u won or lost
}

void UEndGameWidget::OnRestartClicked()
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

void UEndGameWidget::OnExitToMenuClicked()
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

void UEndGameWidget::OnQuitGameClicked()
{
	UE_LOG(LogTemp, Log, TEXT("PM_Widget: QuitGame Clicked"));

	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}