
#include "HUD/TowerDefenceHUD.h"
#include "Player/TopDownPawn.h"

ATowerDefenceHUD::ATowerDefenceHUD()
{
	// not useful for now
}

void ATowerDefenceHUD::BeginPlay()
{
	Super::BeginPlay();

	// Spawn main HUD widget
	if (MainHUDWidgetClass && GetOwningPlayerController())
	{
		MainHUDInstance = CreateWidget<UTDMainHUD>(GetOwningPlayerController(), MainHUDWidgetClass);
		if (MainHUDInstance)
		{
			MainHUDInstance->AddToViewport();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TowerDefenceHUD: Error with MainHUDInstance!"))
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TowerDefenceHUD: No MainHUDWidgetClass or No provide PC!"))
	}
}

void ATowerDefenceHUD::UpdateResources(int32 Gold, int32 Health)
{
	if (MainHUDInstance)
	{
		MainHUDInstance->UpdateResources(Gold, Health);
	}
}

void ATowerDefenceHUD::UpdateMode(EGameMode NewMode)
{
	if (MainHUDInstance)
	{
		MainHUDInstance->UpdateMode(NewMode);
	}
}

void ATowerDefenceHUD::InitializeBindings(APlayerResourceState* InPlayerResources, ABuildManager* InBuildManager)
{
	PlayerResources = InPlayerResources;
	BuildManagerRef = InBuildManager;

	if (PlayerResources)
	{
		PlayerResources->OnResourcesChanged.AddDynamic(this, &ATowerDefenceHUD::UpdateResources);
		UpdateResources(PlayerResources->GetGoldAmount(), PlayerResources->GetHealthAmount()); // Initial update
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TowerDefenceHUD: InitializeBindings received null PlayerResources!"));
	}

	if (BuildManagerRef)
	{
		BuildManagerRef->OnModeChanged.AddDynamic(this, &ATowerDefenceHUD::UpdateMode);
		UpdateMode(BuildManagerRef->CurrentMode); // Initial update
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TowerDefenceHUD: InitializeBindings received null BuildManagerRef!"));
	}
}


