// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeTreeUI.h"

#include "Components/Button.h"

void UUpgradeTreeUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Option1)
	{
		Option1->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption1Clicked);
	}

	if (Option2)
	{
		Option2->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption2Clicked);
	}
	
	if (Option3)
	{
		Option3->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption3Clicked);
	}

	if (Close)
	{
		Close->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnCloseClicked);
	}
}

void UUpgradeTreeUI::OnOption1Clicked()
{
	UE_LOG(LogTemp, Log, TEXT("UpgradeTreeUI: OnOption1Clicked"));
}

void UUpgradeTreeUI::OnOption2Clicked()
{
	UE_LOG(LogTemp, Log, TEXT("UpgradeTreeUI: OnOption2Clicked"));
}

void UUpgradeTreeUI::OnOption3Clicked()
{
	UE_LOG(LogTemp, Log, TEXT("UpgradeTreeUI: OnOption3Clicked"));
}

void UUpgradeTreeUI::OnCloseClicked()
{
	RemoveFromParent();	
}

