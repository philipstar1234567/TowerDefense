

#include "MainMenuPlayerController.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (MainMenuWidgetClass)
	{
		UUserWidget* Menu = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

		if (Menu)
		{
			Menu->AddToViewport();

			// Show cursor
			bShowMouseCursor = true;

			// input mode
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Menu->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			SetInputMode(InputMode);
		}
	}
}
