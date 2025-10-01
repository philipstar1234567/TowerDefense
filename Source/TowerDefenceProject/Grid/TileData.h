
#pragma once

#include "CoreMinimal.h"
#include "TileData.generated.h"

UENUM(BlueprintType)
enum class ETileState : uint8
{
	Default, // Grey/white
	Buildable, // Green
	Occupied, // Red
};

USTRUCT(BlueprintType)
struct FTileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D GridLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstanceIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETileState TileState = ETileState::Default;
};