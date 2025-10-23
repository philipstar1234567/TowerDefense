
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Grid/BuildManager.h"
#include "Player/TopDownPawn.h"
#include "TDMainHUD.generated.h"

UCLASS()
class TOWERDEFENCEPROJECT_API UTDMainHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// called for widget create & remove
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Bind UI
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;

	UPROPERTY(meta = (BindWidget))
	UButton* BuildButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SellButton;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* TowerSelectionPanel;

	// Functions
	UFUNCTION(BlueprintCallable)
	void UpdateResources(int32 Gold, int32 Health);

	UFUNCTION(BlueprintCallable)
	void UpdateMode(EGameMode NewMode);

protected:
	// Bind button clicks
	UFUNCTION()
	void OnBuildButtonClicked();

	UFUNCTION()
	void OnSellButtonClicked();

private:
	// Need pawn reference to do something on pawn using button clicks
	UPROPERTY()
	ATopDownPawn* PlayerPawn;

};
