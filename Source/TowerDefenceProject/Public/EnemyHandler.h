#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyHandler.generated.h"

class AGridManager;
class AEnemyBase;

/**
 * Centralized handler for enemy pathfinding and related services.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API AEnemyHandler : public AActor
{
    GENERATED_BODY()

public:
    AEnemyHandler();

    virtual void BeginPlay() override;

    /** Assign the GridManager in editor or it will try to auto-find in BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AGridManager* GridManager = nullptr;

    /** Finds a path from StartWorld to EndWorld (tile-aware). Returns true and fills OutPath if successful. */
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindPath(const FVector& StartWorld, const FVector& EndWorld, TArray<FVector>& OutPath);

    /** Register/unregister enemies to be notified on grid changes (optional feature). */
    void RegisterEnemy(AEnemyBase* Enemy);
    void UnregisterEnemy(AEnemyBase* Enemy);
    void NotifyGridChanged();

protected:
    // Internal A* node
    struct FPathNode
    {
        FIntPoint Coord;
        int32 ParentIndex = -1;
        float G = 0.f;
        float H = 0.f;
        float F() const { return G + H; }
    };

    /** Registered enemies */
    TArray<TWeakObjectPtr<AEnemyBase>> RegisteredEnemies;

    bool IsTileWalkable(int32 X, int32 Y) const;
    TArray<FIntPoint> GetNeighbors4(const FIntPoint& P) const;
};
