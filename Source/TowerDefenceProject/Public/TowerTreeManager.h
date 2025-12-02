// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerNode.h"
#include "TowerTreeManager.generated.h"

class ATestTower;
class UTowerNode;

UCLASS()
class TOWERDEFENCEPROJECT_API ATowerTreeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATowerTreeManager();

	UFUNCTION()
	void GoToNode(ATestTower* Tower, UTowerNode* NextNode);

	void UpgradeTower(ATestTower* Tower);
	
	bool bIsInUpgradeMenu = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
