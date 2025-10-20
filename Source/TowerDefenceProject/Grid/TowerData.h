
#pragma once

#include "CoreMinimal.h"
#include "TowerData.generated.h"

USTRUCT(BlueprintType)
struct FTowerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D GridLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* TowerActor = nullptr;
};