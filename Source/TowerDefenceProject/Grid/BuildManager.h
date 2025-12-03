#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/GridManager.h"
#include "Player/PlayerResourceState.h"
#include "TowerData.h"
#include "Tower.h"
#include "TestTower.h"
#include "BuildManager.generated.h"

/**
 * @brief Gameplay mode states used by the player and BuildManager.
 */
UENUM(BlueprintType)
enum class EGameMode : uint8
{
	None,
	Build,
	Delete
};

/* Delegates */

/** 
* @brief Called when the current game mode is changed. 
* @param NewMode The game mode beeing switched to
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModeChangedDelegate, EGameMode, NewMode);

/**
*@brief Called when the player selects a new tower index.
*@param TowerIndex Used to select which tower to place (currently not used as we only have 1 type)
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTowerSelectedDelegate, int32, TowerIndex);

/**
*@brief Called when a tower is successfully placed.
*@param Location The placed tower location 
*@param Cost The cost of the tower placed
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTowerPlacedDelegate, FVector2D, Location, int32, Cost);

/**
*@brief Called when a tower is deleted from the grid.
*@param Location The deleted tower location
*@param Refund The refund given when selling tower
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTowerDeleteDelegate, FVector2D, Location, int32, Refund);

/**
 * @brief Manages tower placement, tower deletion, preview meshes,
 * tile interactions, and building-related gameplay logic.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API ABuildManager : public AActor
{
	GENERATED_BODY()
	
public:	
	/** @brief Constructor. Initializes components and default values. */
	ABuildManager();
	
	/** @brief Pointer to GridManager */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	AGridManager* GridManager;

	/** @brief List of available tower classes that may be placed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Towers", meta = (ClampMax = 9)); // clamp max towers
	TArray<TSubclassOf<ATestTower>> AvailableTowers;

	/** @brief Index of the currently selected tower type (-1 = none). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	int32 SelectedTowerIndex = -1;

	/** @brief Cost of the currently selected tower. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	int32 SelectedTowerCost = 0;

	/** @brief Current gameplay mode (Build/Delete/None). */
	UPROPERTY(BlueprintReadOnly, Category = "Mode")
	EGameMode CurrentMode = EGameMode::None;

	/** @brief Percentage of cost refunded when deleting towers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Economy")
	float RefundPercentage = 0.7f;

	/** Events */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnModeChangedDelegate OnModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTowerSelectedDelegate OnTowerSelected;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTowerPlacedDelegate OnTowerPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTowerDeleteDelegate OnTowerDeleted;

	/**
	* @brief Selects which tower type to build.
	* @param NewIndex Index in the AvailableTowers array.
	*/
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SelectTowerIndex(int32 NewIndex);

	/**
	* @brief Gets the cost of the currently selected tower.
	* @return Tower build cost.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Building")
	int32 GetSelectedTowerCost() const { return SelectedTowerCost; }

	/**
	* @brief Sets the current build/delete mode.
	* @param NewMode The desired mode.
	*/
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetMode(EGameMode NewMode);

	/**
	* @brief Attempts to place a tower at the hovered tile.
	* @return True on success.
	*/
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool TryPlaceTower();

	/**
	* @brief Attempts to delete a tower at the hovered tile.
	* @return True on success.
	*/
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool TryDeleteTower();

	/** @brief Returns true if build mode is active. */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsBuildModeActive() const { return CurrentMode == EGameMode::Build; }

	/** @brief Returns true if delete mode is active. */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsDeleteModeActive() const { return CurrentMode == EGameMode::Delete; }

	/**
	* @brief Returns the visual state for a given tile based on mode and hover.
	* @param Tile Tile data.
	* @param bIsHovered True if the cursor is over this tile.
	*/
	UFUNCTION(BlueprintCallable, Category = "Building")
	ETileVisualState GetVisualStateForTile(const FTileData& Tile, bool bIsHovered) const;

	/** @brief Debug command: attempts to place a tower. */
	UFUNCTION(Exec, Category = "Debug") // PIE: "DebugPlaceTower"
	void DebugPlaceTower();

	/** @brief Debug command: attempts to delete a tower. */
	UFUNCTION(Exec, Category = "Debug") // PIE: "DebugDeleteTower"
	void DebugDeleteTower();

	/**
	* @brief Sets the grid manager reference.
	* @param InGridManager Grid manager instance.
	*/
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetGridManager(AGridManager* InGridManager);

	/**
	* @brief Sets the player resource handler reference.
	* @param InPlayerResource Player resource state instance.
	*/
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetPlayerResourceState(APlayerResourceState* InPlayerResource);

	/**
	* @brief Notifies BuildManager that the player started/stopped rotating the camera.
	* @param bIsRotating True if rotating.
	*/
	UFUNCTION()
	void OnPlayerRotating(bool bIsRotating);

protected:
	/** @brief Called at game start. */
	virtual void BeginPlay() override;

	/** @brief Per-frame update. */
	virtual void Tick(float DeltaTime) override;

private:
	/** const for a hardcoded Z offset for towers */
	static constexpr float TowerZOffset = 5.0f;

	// States
	FVector2D LastHoveredTile = FVector2D(-1, -1);
	bool bPlayerRotating = false;
	TSubclassOf<ATestTower> SelectedTowerClass;
	ATestTower* PreviewTower = nullptr;
	APlayerResourceState* PlayerResource = nullptr;

	/** @brief List of all towers placed in the world. */
	TArray<FTowerData> PlacedTowers;

	/** @brief Destroys the preview tower actor and nulls the pointer. */
	UFUNCTION()
	void DestroyAndNullPreview();

	/** @brief Spawns the preview tower actor based on selected tower. */
	UFUNCTION()
	void SpawnAndSetupPreview();

	/**
	* @brief Updates the preview tower's world position based on the hovered tile.
	* @param Tile Tile data.
	*/
	UFUNCTION()
	void UpdatePreviewPosition(const FTileData& Tile);

	/**
	* @brief Validates the currently hovered tile.
	* @param OutTile Filled with tile data if valid.
	* @return True if valid.
	*/
	UFUNCTION()
	bool ValidateHoveredTile(FTileData& OutTile);

	/** @brief Clears hover state on previously hovered tile. */
	UFUNCTION()
	void ResetPreviousHover();

	/**
	* @brief Updates a single tile's visual highlight.
	* @param GridCoords Tile coordinate.
	* @param bIsHovered Whether the tile is hovered.
	*/
	UFUNCTION()
	void UpdateHoverVisual(const FVector2D& GridCoords, bool bIsHovered);

	/**
	* @brief Finds the index of a tower at a given grid location.
	* @param GridLocation Tile coordinates.
	* @return Index in PlacedTowers array, or -1 if none.
	*/
	UFUNCTION()
	int32 FindTowerIndex(const FVector2D& GridLocation) const;

	/**
	* @brief Gets tower index for UI/HUD usage.
	* @param GridLocation Tile coordinate.
	* @return Index, or -1 if not found.
	*/
	UFUNCTION(BlueprintPure, Category = "Building")
	int32 GetTowerIndexAt(const FVector2D GridLocation) const; // -1 = not found
	
	/**
	* @brief Calculates how much currency the player gets back when deleting a tower.
	* @param OriginalCost The tower cost.
	* @return Refunded amount.
	*/
	UFUNCTION()
	int32 CalculateRefund(int32 OriginalCost) const;

	/**
	* @brief Performs the actual placement of a tower.
	* @param Tile Tile to place the tower on.
	*/
	UFUNCTION()
	bool PerformPlacement(const FTileData& Tile);

	/**
	 * @brief Performs the actual deletion of a tower.
	 * @param Tile Tile where the tower exists.
	 */
	UFUNCTION()
	bool PerformDeletion(const FTileData& Tile);

	/**
	 * @brief Retrieves tower data by index, or nullptr.
	 * @param Index Tower index.
	 */
	const FTowerData* GetTowerDataByIndex(int32 Index) const
	{
		return (Index != -1 && PlacedTowers.IsValidIndex(Index))
			? &PlacedTowers[Index]
			: nullptr;
	}

	/** @brief Updates hover state, preview tower, and placement logic each tick. */
	UFUNCTION()
	void UpdatePreview();
};
