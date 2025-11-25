// Fill out your copyright notice in the Description page of Project Settings.

#include "TowerTreeManager.h"
#include "TestTower.h"

// Sets default values
ATowerTreeManager::ATowerTreeManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATowerTreeManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATowerTreeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATowerTreeManager::GoToNode(ATestTower* Tower, UTowerNode* NextNode)
{
	if (Tower->CurrentNode->Children.Contains(NextNode))
	{
		Tower->CurrentNode = NextNode;
		UpgradeTower(Tower);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Next node is not a child of the current node!"));
	}
	
}


//Important: When updating FireRate, always call Tower->ResetTimer() or it will not have any effect. And remember to divide for FireRate, not multiply (see code for example)
void ATowerTreeManager::UpgradeTower(ATestTower* Tower)
{
	switch (Tower->CurrentNode->ID)
	{
	case 1:
		Tower->Range *= 2.0f;
		break;
	case 2:
		Tower->FireRate /= 2.0f;
		Tower->ResetTimer();
		break;
	case 3:
		Tower->Strength *= 2.0f;
		break;
	case 4:
		Tower->FireRate /= 1.2f;
		Tower->ResetTimer();
		break;
	case 5:
		Tower->Strength *= 1.2f;
		break;
	case 6:
		Tower->Range *= 1.2f;
		break;
	case 7:
		Tower->Strength *= 1.2f;
		break;
	case 8:
		Tower->Range *= 1.2f;
		break;
	case 9:
		Tower->FireRate /= 1.2f;
		Tower->ResetTimer();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Tried to upgrade to inaccesible node!"));
	}
}
