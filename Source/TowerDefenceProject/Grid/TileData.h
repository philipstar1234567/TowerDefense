
#pragma once

#include "CoreMinimal.h"
#include "TileData.generated.h"

UENUM(BlueprintType)
enum class ETileVisualState : uint8
{
	Default,	// Normal color
	Buildable,	// Green Highlight
	Occupied,	// Red Highlight
	Highlighted, // Yellow?
	Blocked,	// grey?
};

UENUM(BlueprintType)
enum class ETileOccupancyState : uint8
{
	Empty,
	Building, // Might have something like this in the future?
	Tower,
	Path,
	Blocked,
};

USTRUCT(BlueprintType)
struct FTileData
{
	GENERATED_BODY()

	// --- Static Info ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D GridLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstanceIndex = -1;


	// --- Logical State ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETileOccupancyState Occupancy = ETileOccupancyState::Empty;

	// Optional point for easy refence
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> OccupantActor;

	// --- Visual State ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETileVisualState VisualState = ETileVisualState::Default;

	// --- Helper Functions ---
	bool IsBuildable() const
	{
		return Occupancy == ETileOccupancyState::Empty;
	}

	bool IsOccupied() const
	{
		return Occupancy != ETileOccupancyState::Empty && Occupancy != ETileOccupancyState::Blocked;
	}

	void SetOccupant(AActor* NewOccupant, ETileOccupancyState NewState)
	{
		OccupantActor = NewOccupant
;		Occupancy = NewState;
	}

	void ClearOccupant()
	{
		OccupantActor = nullptr;
		Occupancy = ETileOccupancyState::Empty;
	}
};