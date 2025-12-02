// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeTreeUI.h"

#include "TestTower.h"
#include "TowerNode.h"
#include "TowerTreeManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UUpgradeTreeUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	UpgradeOptions = Tower->CurrentNode->Children;

	if (Option1)
	{
		Option1->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption1Clicked);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 1 has no OnClick!"));
	}

	if (Option2)
	{
		Option2->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption2Clicked);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 2 has no OnClick!"));
	}

	if (Option3)
	{
		Option3->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnOption3Clicked);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 3 has no OnClick!"));
	}

	if (Close)
	{
		Close->OnClicked.AddDynamic(this, &UUpgradeTreeUI::OnCloseClicked);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Close has no OnClick!"));
	}
	
	if (Option1Text)
	{
		if (UpgradeOptions.Num() >= 1)
		{
			Option1Text->SetText(UpgradeOptions[0]->DisplayText);
		}
	}
	if (Option2Text)
	{
		if (UpgradeOptions.Num() >= 2)
		{
			Option2Text->SetText(UpgradeOptions[1]->DisplayText);
		}
	}
	if (Option3Text)
	{
		if (UpgradeOptions.Num() >= 3)
		{
			Option3Text->SetText(UpgradeOptions[2]->DisplayText);
		}
	}
	
}

void UUpgradeTreeUI::OnOption1Clicked()
{
	if (Option1Text->Text.EqualTo(Default) == false)
	{
		Tower->TowerTreeManager->GoToNode(Tower, UpgradeOptions[0]);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 1 Clicked"));
		Tower->TowerTreeManager->bIsInUpgradeMenu = false;
		RemoveFromParent();
	}
}

void UUpgradeTreeUI::OnOption2Clicked()
{
	if (Option2Text->Text.EqualTo(Default) == false)
	{
		Tower->TowerTreeManager->GoToNode(Tower, UpgradeOptions[1]);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 2 Clicked"));
		Tower->TowerTreeManager->bIsInUpgradeMenu = false;
		RemoveFromParent();	
	}
}

void UUpgradeTreeUI::OnOption3Clicked()
{
	if (Option3Text->Text.EqualTo(Default) == false)
	{
		Tower->TowerTreeManager->GoToNode(Tower, UpgradeOptions[2]);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Option 3 Clicked"));
		Tower->TowerTreeManager->bIsInUpgradeMenu = false;
		RemoveFromParent();	
	}
}

void UUpgradeTreeUI::OnCloseClicked()
{
	Tower->TowerTreeManager->bIsInUpgradeMenu = false;
	RemoveFromParent();	
}

