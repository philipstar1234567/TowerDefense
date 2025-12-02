#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyHandler.generated.h"

class AGridManager;
class ABuildManager;
class AEnemyBase;

/**
 * @class AEnemyHandler
 * @brief Centralized handler responsible for enemy pathfinding, tile data queries,
 *        and maintaining a registry of active enemies.
 *
 * This class provides A* pathfinding using the grid from AGridManager, notifies
 * enemies when the navigation grid changes, and assists enemies in locating
 * required managers if references are not yet initialized.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API AEnemyHandler : public AActor
{
    GENERATED_BODY()

public:

    /** @brief Constructor. Initializes default state. */
    AEnemyHandler();

    /** @brief Called when the game begins play. */
    virtual void BeginPlay() override;

    /** @brief Reference to the world grid used for pathfinding. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AGridManager* GridManager = nullptr;

    /** @brief Reference to BuildManager, used for detecting world obstacles. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ABuildManager* BuildManager = nullptr;

    /**
     * @brief Computes a path between two world positions.
     *
     * Performs tile-aware A* search using the grid provided by GridManager.
     *
     * @param StartWorld Starting world position.
     * @param EndWorld Target world position.
     * @param OutPath Output list of world positions forming the navigation path.
     * @return true if a valid path was found, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindPath(const FVector& StartWorld, const FVector& EndWorld, TArray<FVector>& OutPath);

    /**
     * @brief Registers an enemy with this handler.
     *
     * Stored as a weak pointer to avoid invalid references.
     *
     * @param Enemy Pointer to enemy instance to add.
     */
    void RegisterEnemy(AEnemyBase* Enemy);

    /**
     * @brief Unregisters a previously registered enemy.
     *
     * @param Enemy Enemy to remove from registry.
     */
    void UnregisterEnemy(AEnemyBase* Enemy);

    /**
     * @brief Called when a tile's traversal cost changes.
     *
     * Used to notify enemies that their path may be invalid and must be updated.
     *
     * @param loc Tile coordinate affected.
     * @param cost New movement cost at this tile.
     */
    UFUNCTION()
    void NotifyGridChanged(FVector2D loc, int32 cost);

    /**
     * @brief Attempts to locate the GridManager if the reference is missing.
     *
     * Called periodically until successful.
     */
    UFUNCTION()
    void TryFindGridManager1();

    /**
     * @brief Attempts to locate the BuildManager if the reference is missing.
     *
     * Called periodically until successful.
     */
    UFUNCTION()
    void TryFindBuildManager();

protected:

    /**
     * @struct FPathNode
     * @brief Internal representation of an A* node for pathfinding.
     *
     * Stores tile coordinates, parent index for path reconstruction,
     * and G/H costs for the A* algorithm.
     */
    struct FPathNode
    {
        /** Tile coordinate on the grid. */
        FIntPoint Coord;

        /** Index of the parent node in the open/closed list. */
        int32 ParentIndex = -1;

        /** Cost from start node to this node. */
        float G = 0.f;

        /** Heuristic cost from this node to the target. */
        float H = 0.f;

        /** @brief Combined F-score for A* (G + H). */
        float F() const { return G + H; }
    };

    /** @brief List of active enemies registered to this handler. */
    TArray<TWeakObjectPtr<AEnemyBase>> RegisteredEnemies;

    /**
     * @brief Checks if a tile coordinate can be walked on.
     *
     * @param X Tile X coordinate.
     * @param Y Tile Y coordinate.
     * @return true if walkable, false otherwise.
     */
    bool IsTileWalkable(int32 X, int32 Y) const;

    /**
     * @brief Returns the four cardinal neighbors of a tile coordinate.
     *
     * Used for grid-based A* path expansion.
     *
     * @param P Tile coordinate.
     * @return Array of neighbor tile coordinates.
     */
    TArray<FIntPoint> GetNeighbors4(const FIntPoint& P) const;

    /** @brief Timer used to repeatedly attempt locating GridManager. */
    FTimerHandle TryFindGridManagerHandle1;

    /** @brief Timer used to repeatedly attempt locating BuildManager. */
    FTimerHandle TryFindBuildManagerHandle;
};
