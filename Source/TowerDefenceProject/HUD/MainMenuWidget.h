
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

// Forward declarations for pointer stuff
class UButton;
class UKismetSystemLibrary;


UCLASS()
class TOWERDEFENCEPROJECT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// if needed: call after creating widget, pass data if needed
	void Setup();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	/*-- Bindings --*/
	UPROPERTY(meta = (BindWidget))
	class UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	/*-- Button callbacks --*/
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnQuitClicked();

	// Configurable level to lead (set in BP)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu")
	FName LevelToLoad = NAME_None;
};
