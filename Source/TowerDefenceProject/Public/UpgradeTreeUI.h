// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/PlayerResourceState.h"
#include "UpgradeTreeUI.generated.h"

class UTowerNode;
class UTextBlock;
class UButton;
class ATestTower;
class APlayerResourceState;

/**
 * @brief Class for the UI that shows up when a tower is clicked on
 * 
 * Relies on the WBP_UpgradeTreeUI class for widgets and such. Cannot
 * currently support more than three upgrade options.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API UUpgradeTreeUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/**
	 * @brief Custom constructor to be called when creating widget
	 * 
	 * Exists instead of the usual NativeConstruct as we
	 * ran into a lot of issues with the normal constructor.
	 * Problem was the constructor needed a tower to be made but
	 * you couldn't give the class a tower to use in the constructor
	 * before the constructor was done. This one just takes it as
	 * a parameter. Must always be called when instantiating.
	 * 
	 * @param InTower The tower to be upgraded in the UI
	 */
	void NewConstruct(ATestTower* InTower);
	
	// Tower to be upgraded in the UI
	ATestTower* Tower;
	
	// Child nodes of the node that tower is currently on
	TArray<UTowerNode*> UpgradeOptions;
	
	//Option 1, 2 and 3 are the actual buttons while Option1Text, 2Text and 3Text is what is displayed on top of the button
	
	UPROPERTY(meta = (BindWidget))
	UButton* Option1;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Option1Text;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Option2;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Option2Text;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Option3;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Option3Text;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Close;
	
	// Reference to class that holds the player's gold, to check if you have enough gold for the upgrade
	UPROPERTY(VisibleAnywhere)
	APlayerResourceState* PlayerResourceState;
	
	// Default text to be displayed if there is no available option for a slot
	FText Default = FText::FromString(TEXT("X"));
	
	UFUNCTION()
	//Checks if the player has enough gold, if so tells ATowerTreeManager to upgrade and closes
	void OnOption1Clicked();
	
	//Checks if the player has enough gold, if so tells ATowerTreeManager to upgrade and closes
	UFUNCTION()
	void OnOption2Clicked();
	
	//Checks if the player has enough gold, if so tells ATowerTreeManager to upgrade and closes
	UFUNCTION()
	void OnOption3Clicked();
	
	//Closes the goddamn menu (who would've thought)
	UFUNCTION()
	void OnCloseClicked();
};
