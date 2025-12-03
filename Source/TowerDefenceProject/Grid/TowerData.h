
#pragma once

#include "CoreMinimal.h"
#include "TowerData.generated.h"

/**
* @brief Structure that holds tower data like location, tower pointer, tower cost
*/
USTRUCT(BlueprintType)
struct FTowerData
{
	GENERATED_BODY()

	/** @brief GridLocation on the grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D GridLocation;

	/** @brief Pointer to the tower placed on it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> TowerActor = nullptr;

	/** @brief Tower cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cost = 0;
};