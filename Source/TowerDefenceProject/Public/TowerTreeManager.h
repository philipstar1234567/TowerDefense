// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerNode.h"
#include "TowerTreeManager.generated.h"

class ATestTower;
class UTowerNode;

/**
 * @brief Manages the upgrades for the towers.
 * 
 * Works in conjunction with the UUpgradeTreeUI class. This
 * class specifically applies the actual upgrades after getting the
 * go-ahead from the UI.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API ATowerTreeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATowerTreeManager();

	/**
	 * @brief Checks if the next node is a valid node
	 * 
	 * Note: Doesn't actually apply the upgrade, but if the
	 * node is a child of the tower's current node, this function will
	 * call UpgradeTower(), which then applies the stat change
	 * 
	 * @param Tower The tower requesting an upgrade
	 * @param NextNode The node the tower wants to upgrade to in the tree
	 */
	UFUNCTION()
	void GoToNode(ATestTower* Tower, UTowerNode* NextNode);

	/**
	 * @brief Appliees stat change according to the tower's current node
	 * 
	 * This function assumes it's being called after GoToNode() has just
	 * changed the Tower's current node, and therefore doesn't have a
	 * node parameter.
	 * 
	 * @param Tower The tower being upgraded
	 */
	void UpgradeTower(ATestTower* Tower);
	
	//Bool for if the player is in the upgrade menu, used to stop the player from opening multiple menus on top of each other.
	bool bIsInUpgradeMenu = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
