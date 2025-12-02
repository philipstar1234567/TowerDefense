// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TowerNode.generated.h"

/**
 * 
 */
UCLASS()
class TOWERDEFENCEPROJECT_API UTowerNode : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "TowerNode")
	FName Name;

	UPROPERTY(EditAnywhere, Category = "TowerNode")
	int8 ID;

	UPROPERTY(EditAnywhere, Category = "TowerNode")
	TArray<UTowerNode*> Children;
	
	UPROPERTY(EditAnywhere, Category = "TowerNode")
	FText DisplayText;
};
