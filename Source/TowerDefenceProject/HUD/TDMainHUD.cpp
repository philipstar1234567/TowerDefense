
#include "HUD/TDMainHUD.h"

void UTDMainHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	// bind button click events
	if (BuildButton)
	{
		BuildButton->OnClicked.AddDynamic(this, &UTDMainHUD::OnBuildButtonClicked);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TDMainHUD: No BuildButton!"));
	}

	if (SellButton)
	{
		SellButton->OnClicked.AddDynamic(this, &UTDMainHUD::OnSellButtonClicked);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TDMainHUD: No SellButton!"));
	}

	// Cache pawn reference
	if (APlayerController* PC = GetOwningPlayer())
	{
		PlayerPawn = Cast<ATopDownPawn>(PC->GetPawn());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TDMainHUD: No PC!"));
	}

	// Initialize UI
	if (TowerSelectionPanel)
	{
		TowerSelectionPanel->SetVisibility(ESlateVisibility::Hidden); // hide on start
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TDMainHUD: No TowerSelectionPanel!"));
	}
}

void UTDMainHUD::NativeDestruct()
{
	// Clean bindings
	if (BuildButton)
	{
		BuildButton->OnClicked.RemoveDynamic(this, &UTDMainHUD::OnBuildButtonClicked);
	}
	if (SellButton)
	{
		SellButton->OnClicked.RemoveDynamic(this, &UTDMainHUD::OnSellButtonClicked);
	}
	Super::NativeDestruct();
}

void UTDMainHUD::UpdateResources(int32 Gold, int32 Health)
{
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("Health: %d"), Health)));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TDMainHUD: No HealthText!"))
	}

	if (GoldText)
	{
		GoldText->SetText(FText::FromString(FString::Printf(TEXT("Gold: %d"), Gold)));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TDMainHUD: No GoldText!"))
	}
}

void UTDMainHUD::UpdateMode(EGameMode NewMode)
{
	if (TowerSelectionPanel)
	{
		TowerSelectionPanel->SetVisibility(NewMode == EGameMode::Build
			? ESlateVisibility::Visible : ESlateVisibility::Hidden); // Can also be visible normally or during Selling. can change later
	}

	// Highlight active button
	if (BuildButton)
	{
		BuildButton->SetIsEnabled(NewMode != EGameMode::Build);
	}
	if (SellButton)
	{
		SellButton->SetIsEnabled(NewMode != EGameMode::Delete);
	}
}

void UTDMainHUD::OnBuildButtonClicked()
{
	if (PlayerPawn)
	{
		PlayerPawn->ToggleBuildMode();
	}
}

void UTDMainHUD::OnSellButtonClicked()
{
	if (PlayerPawn)
	{
		PlayerPawn->ToggleDeleteMode();
	}
}

