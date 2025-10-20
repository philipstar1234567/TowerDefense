
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/TileData.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GridManager.generated.h"

UCLASS()
class TOWERDEFENCEPROJECT_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:
	AGridManager();

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* TileMesh;

	// Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float TileSize;

	// HelperFunction

	UFUNCTION()
	bool SetTileVisual(int32 X, int32 Y, ETileVisualState NewVisualState);

	UFUNCTION()
	bool SetTileOccupancy(int32 X, int32 Y, ETileOccupancyState NewOccupancy);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool WorldToGrid(const FVector& WorldLocation, FVector2D& OutGridCoords) const;

	UFUNCTION(BlueprintCallable)
	bool GetTileSafe(int32 X, int32 Y, FTileData& OutTile) const;

	// GridArray
	TArray<TArray<FTileData>> TileGrid;

protected:
	virtual void BeginPlay() override;

private:

	UFUNCTION()
	void GenerateGrid();

	UFUNCTION()
	void SpawnTileVisuals();

	UFUNCTION()
	void UpdateTileVisual(const FTileData& Tile);
};
