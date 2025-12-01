
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "EndGameWidget.generated.h"

// Forward declarations for pointer stuff
class UButton;
class UKismetSystemLibrary;


UCLASS()
class TOWERDEFENCEPROJECT_API UEndGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/*-- Bindings --*/
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Headline;

	UPROPERTY(meta = (BindWidget))
	class UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ExitToMenuButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitGameButton;

	/*-- Button Callbacks --*/
	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnExitToMenuClicked();

	UFUNCTION()
	void OnQuitGameClicked();

	// properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu")
	FName MainMenuLevel = NAME_None; // to open main menu

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu")
	FName CurrentLevel = NAME_None; // to restart the game

	UPROPERTY(EditAnywhere, Category = "Menu")
	bool bWinning;
};
