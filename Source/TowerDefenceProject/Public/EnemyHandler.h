#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/TileData.h"
#include "EnemyHandler.generated.h"

class AGridManager;
class AEnemyBase;

/**
 * Centralized handler for enemy pathfinding and related services.
 * - Uses AGridManager's TileGrid and WorldToGrid to compute paths (A*).
 * - Allows enemies to register, unregister.
 * - Can notify all registered enemies to recalc their path when the grid changes.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API AEnemyHandler : public AActor
{
    GENERATED_BODY()

public:
    AEnemyHandler();

    virtual void BeginPlay() override;

    /** The grid manager that owns TileGrid and WorldToGrid. Auto-found in BeginPlay if not set. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AGridManager* GridManager = nullptr;

    /** Find a path between StartWorld and EndWorld. Returns true and fills OutPath if a path is found. */
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindPath(const FVector& StartWorld, const FVector& EndWorld, TArray<FVector>& OutPath);

    /** Register an enemy so this handler can request it to re-path later. */
    void RegisterEnemy(AEnemyBase* Enemy);

    /** Unregister an enemy (call on destroy/endplay). */
    void UnregisterEnemy(AEnemyBase* Enemy);

    /** Notify the handler that the grid changed (e.g., tower placed/removed). This asks all registered enemies to recalc path. */
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void NotifyGridChanged();

protected:
    // A simple node used for A*
    struct FPathNode
    {
        FIntPoint Coord;
        int32 ParentIndex; // index in Nodes array (-1 = none)
        float G; // cost from start
        float H; // heuristic to goal

        FPathNode() : Coord(0, 0), ParentIndex(-1), G(0.f), H(0.f) {}
        float F() const { return G + H; }
    };

    /** Registered enemies (weak ptrs so we don't keep dangling refs). */
    TArray<TWeakObjectPtr<AEnemyBase>> RegisteredEnemies;

    /** A* helpers */
    bool IsTileWalkable(int32 X, int32 Y) const;
    TArray<FIntPoint> GetNeighbors4(const FIntPoint& P) const;
};
