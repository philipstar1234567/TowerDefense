
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/TileData.h"
#include "GridManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileVisualChanged, int32, X, int32, Y);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileOccupancyChanged, int32, X, int32, Y);

UENUM(BlueprintType)
enum class ETileVisualState : uint8
{
	Default,
	Buildable,
	Occupied,
	Blocked,
	Path,
	Highlighted,
	Spawn,
	Goal
};

UCLASS()
class TOWERDEFENCEPROJECT_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:
	AGridManager();

	/* Grid size - Set in editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridSizeX = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridSizeY = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float TileSize = 100.f;

	/* Goal and spawn location */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Course")
	FVector2D SpawnTile = FVector2D(-1, -1);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Course")
	FVector2D GoalTile = FVector2D(-1, -1);

	/* Visual material for each state ??? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TMap<ETileVisualState, UMaterialInterface*> VisualMaterials;

	// TileMesh
	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	class UStaticMesh* TileMeshAsset;

	/* Events */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTileVisualChanged OnTileVisualChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTileOccupancyChanged OnTileOccupancyChanged;

	// Public API
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void InitializeGrid();

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsValidTile(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetGridWidth() const { return GridSizeX; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetGridHeight() const { return GridSizeY; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool GetTileSafe(int32 X, int32 Y, FTileData& OutTile) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetTileWorldLocation(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool WorldToGrid(const FVector& WorldLocation, FVector2D& OutGrid) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool SetTileOccupant(int32 X, int32 Y, AActor* NewOccupant);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetTileVisual(int32 X, int32 Y, ETileVisualState NewState);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool SetTileOccupancy(int32 X, int32 Y, ETileOccupancyState NewOccupancy, AActor* NewOccupant = nullptr);

	/* Debug console commands */
	UFUNCTION(Exec) void DebugPrintGrid() const;
	UFUNCTION(Exec) void DebugSetVisual(int32 X, int32 Y, int32 State);

	struct FPathNode
	{
		int32 X;
		int32 Y;
		float GCost; // Distance from start
		float HCost; // Heuristic to goal
		FPathNode* Parent;

		float GetFCost() const { return GCost + HCost; }

		FPathNode(int32 InX, int32 InY, float InG, float InH, FPathNode* InParent)
			: X(InX), Y(InY), GCost(InG), HCost(InH), Parent(InParent) {
		}
	};

	/* Data */
	UPROPERTY()
	TArray<FTileData> TileGrid; // 1D Array

protected:
	virtual void BeginPlay() override;

private:
	/* Internal Helpers */
	void CreateTileMesh(int32 X, int32 Y);
	void UpdateTileVisualInternal(int32 X, int32 Y);
	void GenerateCourse();
	int32 GetTileIndex(int32 X, int32 Y) const;
	//ETileVisualState GetVisualStateForOccupancy(ETileOccupancyState Occupancy) const;

	

	UPROPERTY()
	TMap<int32, UStaticMeshComponent*> TileMeshes; // Index -> mesh

	
};
