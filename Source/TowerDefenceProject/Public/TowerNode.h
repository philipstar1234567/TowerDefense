// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TowerNode.generated.h"

/**
 * @brief Template for nodes in the tower upgrade data tree.
 * 
 * Instances are made by creating new data asset in the editor. Made 
 * to be read by ATowerTreeManager and UUpgradeTreeUI.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API UTowerNode : public UDataAsset
{
	GENERATED_BODY()

public:
	//Name of node (goes unused in code)
	UPROPERTY(EditAnywhere, Category = "TowerNode")
	FName Name;

	//ID used by ATowerTreeManager
	UPROPERTY(EditAnywhere, Category = "TowerNode")
	int8 ID;
	
	UPROPERTY(EditAnywhere, Category = "TowerNode")
	TArray<UTowerNode*> Children;
	
	//Text that's displayed on the upgrade buttons
	UPROPERTY(EditAnywhere, Category = "TowerNode")
	FText DisplayText;
};
