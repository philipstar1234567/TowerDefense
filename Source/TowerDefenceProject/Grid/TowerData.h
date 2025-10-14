
#pragma once

#include "CoreMinimal.h"
#include "TowerData.generated.h"

USTRUCT(BlueprintType)
struct FTowerData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D GridLocation;

	UPROPERTY()
	AActor* TowerActor = nullptr; // Reference to the tower
};