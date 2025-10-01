
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileData.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GridManager.generated.h"

UCLASS()
class TOWERDEFENCEPROJECT_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:
	// Constructor
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

	UFUNCTION()
	bool SetTileState(int32 X, int32 Y, ETileState NewState);

	// GridArray
	TArray<TArray<FTileData>> TileGrid;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	UFUNCTION()
	void GenerateGrid();

	UFUNCTION()
	void SpawnTileVisuals();

	UFUNCTION()
	void UpdateTileVisual(const FTileData& Tile);
};
