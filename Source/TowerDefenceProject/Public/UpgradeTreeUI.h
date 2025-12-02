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
 * 
 */
UCLASS()
class TOWERDEFENCEPROJECT_API UUpgradeTreeUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	ATestTower* Tower;
	TArray<UTowerNode*> UpgradeOptions;
	
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
	
	UPROPERTY(VisibleAnywhere)
	APlayerResourceState* PlayerResourceState;
	
	FText Default = FText::FromString(TEXT("X"));
	
	UFUNCTION()
	void OnOption1Clicked();
	
	UFUNCTION()
	void OnOption2Clicked();
	
	UFUNCTION()
	void OnOption3Clicked();
	
	UFUNCTION()
	void OnCloseClicked();
};
