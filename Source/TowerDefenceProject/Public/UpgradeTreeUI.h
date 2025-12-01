// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeTreeUI.generated.h"

class UButton;
class ATestTower;

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
	
	UPROPERTY(meta = (BindWidget))
	UButton* Option1;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Option2;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Option3;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Close;
	
	UFUNCTION()
	void OnOption1Clicked();
	
	UFUNCTION()
	void OnOption2Clicked();
	
	UFUNCTION()
	void OnOption3Clicked();
	
	UFUNCTION()
	void OnCloseClicked();
};
