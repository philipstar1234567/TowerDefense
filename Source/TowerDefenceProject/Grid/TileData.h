
#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "TileData.generated.h"

UENUM(BlueprintType)
enum class ETileOccupancyState : uint8
{
	Empty,
	Tower,
	Path,		// For enemies
	Blocked,	// Unbuildable tiles
	Spawn,		// Enemy spawn/start
	Goal		// Enemy goal/end
};

USTRUCT(BlueprintType)
struct FTileData
{
	GENERATED_BODY()

	// === STATIC INFO ===
	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	FVector2D GridLocation = FVector2D(-1, -1);

	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	FVector WorldLocation = FVector::ZeroVector;

	// === LOGICAL STATE ===
	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	ETileOccupancyState Occupancy = ETileOccupancyState::Empty;

	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	TWeakObjectPtr<AActor> OccupantActor;

	// === INTERNAL STATE === (GridManager only)
	UPROPERTY()
	uint8 InternalVisualState = 0; // Index -> ETileVisualState

	// === QUERIES === (pure functions)
	bool IsEmpty() const { return Occupancy == ETileOccupancyState::Empty;  }

	bool IsBuildable() const { return Occupancy == ETileOccupancyState::Empty; }

	bool IsOccupied() const
	{ 
		return	Occupancy == ETileOccupancyState::Tower ||
			Occupancy == ETileOccupancyState::Spawn ||
			Occupancy == ETileOccupancyState::Goal;
	}

	bool IsPathBlocked() const { return Occupancy == ETileOccupancyState::Blocked; }
};