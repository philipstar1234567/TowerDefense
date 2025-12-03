#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/TileData.h"
#include "EnemyHandler.h"
#include "GridManager.generated.h"

/**
*@brief Called when a Tile visual changed
*@param X The X location in the grid
*@param Y The Y location in the grid
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileVisualChanged, int32, X, int32, Y);

/**
*@brief Called when something changed on a tile regarding occupation.
*@param X The X location in the grid
*@param Y The Y location in the grid
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileOccupancyChanged, int32, X, int32, Y);


/**
*@brief Visual states used by the tiles to determine visual and some actions
*/
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

/**
*@brief Has the logic for managing the Grid, including tiles and tile data,
* Tower placement rules, pathfinding helpers for enemies, visuals and selection of spawn and goal
*/
UCLASS()
class TOWERDEFENCEPROJECT_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:
	/** @brief Constructor. Initializzs defaults*/
	AGridManager();

	/** @brief Enemy handler reference used for pathfinding and other logic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	AEnemyHandler* EnemyHandler = nullptr;

	/** @brief Number of tiles along the X */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridSizeX = 10;

	/** @brief Number of tiles along the Y */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridSizeY = 10;

	/** @brief Size value of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float TileSize = 100.f;

	/** @brief location for the SpawnTile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Course")
	FVector2D SpawnTile = FVector2D(-1, -1);

	/** @brief Location for the GoalTile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Course")
	FVector2D GoalTile = FVector2D(-1, -1);

	/** @brief Material for each state for the tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TMap<ETileVisualState, UMaterialInterface*> VisualMaterials;

	/** @brief Static mesh for the tiles */
	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	class UStaticMesh* TileMeshAsset;

	/** Events */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTileVisualChanged OnTileVisualChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTileOccupancyChanged OnTileOccupancyChanged;

	/** 
	* @brief Initializes the grid
	*/
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void InitializeGrid();

	/**
	* @brief Checks if you can place towers at location
	* @param Tile Tile structure with values
	* @return True if tile is buildable
	*/
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool CanPlaceTowerAt(FIntPoint Tile);

	/**
	* @brief checks if the tile is valid
	* @param X The location on X
	* @param Y The location on Y
	* @return True if valid
	*/
	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsValidTile(int32 X, int32 Y) const;

	/**
	* @brief Gets the grid width / X size
	* @return gridsize X
	*/
	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetGridWidth() const { return GridSizeX; }

	/**
	* @brief Gets the grid height / Y size
	* @return gridsize Y
	*/
	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetGridHeight() const { return GridSizeY; }

	/**
	* @brief Gets the grid height / Y size
	* @param X The X location on the grid
	* @param Y The Y locaition on the grid
	* @param OutTile give a tile and it will return with values
	* @return True if given valid locations
	*/
	UFUNCTION(BlueprintPure, Category = "Grid")
	bool GetTileSafe(int32 X, int32 Y, FTileData& OutTile) const;

	/**
	* @brief Gets the world location based on grid coordinates
	* @param X the x coord on grid
	* @param Y the y coord on grid 
	* @return FVector with the tiles world location (x,y,z)
	*/
	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetTileWorldLocation(int32 X, int32 Y) const;

	/**
	* @brief Finds closest tile from world location
	* @param WorldLocation The world location...
	* @param OutGrid Tile coords
	* @return True if valid coords given
	*/
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool WorldToGrid(const FVector& WorldLocation, FVector2D& OutGrid) const;

	/**
	* @brief Sets given the tile occupant on given
	* @param X The X grid coord
	* @param Y The Y grid coord
	* @param NewOccupant The Occupant that will occupy the tile
	* @return True if given coords valid and new occupant was valid
	*/
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool SetTileOccupant(int32 X, int32 Y, AActor* NewOccupant);

	/**
	* @brief Sets tile on coords to new visual
	* @param X the grid x coord
	* @param Y the grid Y coord
	* @param NewState The new visual state from struct ETileVisualState
	*/
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetTileVisual(int32 X, int32 Y, ETileVisualState NewState);

	/**
	* @brief Sets tile occupancy based on coords, Occupancy state from struct and the aactor beeing placed
	* @param X the x grid coord
	* @param Y the y grid coord
	* @param NewOccupancy The new occupancy state from struct
	* @param NewOccupant The new occupant for the tile
	* @return True if valid coords, valid occupancy state, valid Occpupant
	*/
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool SetTileOccupancy(int32 X, int32 Y, ETileOccupancyState NewOccupancy, AActor* NewOccupant = nullptr);

	/* Debug console commands */
	// cannot use, does not work on an AActor...
	UFUNCTION(Exec) void DebugPrintGrid() const;
	UFUNCTION(Exec) void DebugSetVisual(int32 X, int32 Y, int32 State);

	/** @brief structure for info for pathfinding */
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

	/**
	* @brief Array with TileData info 
	*/
	UPROPERTY()
	TArray<FTileData> TileGrid; // 1D Array

protected:
	/** @brief The function that runs on begin play */
	virtual void BeginPlay() override;

private:
	/** 
	* @brief Creates the mesh on location based on grid coords given
	* @param X The x grid coord
	* @param Y The y grid coord
	*/
	void CreateTileMesh(int32 X, int32 Y);

	/**
	* @brief Updates the visual for the tile on given coords
	* @param X The x grid coord
	* @param Y the y grid coord
	*/
	void UpdateTileVisualInternal(int32 X, int32 Y);

	/** @brief Generates the course */
	void GenerateCourse();

	/** 
	* @briefGets the tile index from array using coords
	* @param X The x grid coord
	* @param Y The y grid coord
	* @return Tile index based on coords
	*/
	int32 GetTileIndex(int32 X, int32 Y) const;
	//ETileVisualState GetVisualStateForOccupancy(ETileOccupancyState Occupancy) const;

	/** @brief A map of meshes*/
	UPROPERTY()
	TMap<int32, UStaticMeshComponent*> TileMeshes; // Index -> mesh
};
