// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeTreeUI.h"

#include "Components/Button.h"

void UUpgradeTreeUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	//Option1->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption1Clicked);
	//Option2->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption2Clicked);
	//Option3->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption3Clicked);
	//Close->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnCloseClicked);
}

void UUpgradeTreeUI::OnOption1Clicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 1 Clicked"));
}

void UUpgradeTreeUI::OnOption2Clicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 2 Clicked"));
}

void UUpgradeTreeUI::OnOption3Clicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 3 Clicked"));
}

void UUpgradeTreeUI::OnCloseClicked()
{
	RemoveFromParent();	
}

