#include "EnemyHandler.h"
#include "Kismet/GameplayStatics.h"
#include "Grid/GridManager.h"
#include "Grid/BuildManager.h"
#include "EnemyBase.h"
#include "Algo/Reverse.h"

/**
 * @brief Constructor.
 *
 * Disables ticking (the handler uses timers instead). Manager lookups
 * occur via delayed search in BeginPlay().
 */
AEnemyHandler::AEnemyHandler()
{
    PrimaryActorTick.bCanEverTick = false;
}

/**
 * @brief Called once the game starts.
 *
 * Begins repeated attempts to locate both GridManager and BuildManager.
 * These searches are deferred because these managers may not exist or
 * be initialized at level load time.
 */
void AEnemyHandler::BeginPlay()
{
    Super::BeginPlay();

    // Periodically search for GridManager
    GetWorldTimerManager().SetTimer(
        TryFindGridManagerHandle1,
        this,
        &AEnemyHandler::TryFindGridManager1,
        0.1f,
        true
    );

    // Periodically search for BuildManager
    GetWorldTimerManager().SetTimer(
        TryFindBuildManagerHandle,
        this,
        &AEnemyHandler::TryFindBuildManager,
        0.1f,
        true
    );
}

/**
 * @brief Attempts to locate the GridManager in the world.
 *
 * When found, registers this EnemyHandler with it and stops the timer.
 */
void AEnemyHandler::TryFindGridManager1()
{
    if (GridManager == nullptr)
    {
        GridManager = Cast<AGridManager>(
            UGameplayStatics::GetActorOfClass(this, AGridManager::StaticClass())
        );

        if (GridManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: Found GridManager after spawn."));
            GridManager->EnemyHandler = this;

            GetWorldTimerManager().ClearTimer(TryFindGridManagerHandle1);
        }
    }
}

/**
 * @brief Attempts to locate the BuildManager in the world.
 *
 * When found, hooks into BuildManager's tower placement/removal events
 * to force enemies to recalculate their paths.
 */
void AEnemyHandler::TryFindBuildManager()
{
    if (BuildManager == nullptr)
    {
        BuildManager = Cast<ABuildManager>(
            UGameplayStatics::GetActorOfClass(this, ABuildManager::StaticClass())
        );

        if (BuildManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: Found BuildManager after spawn."));

            // React to grid-modifying actions (placing/removing towers)
            BuildManager->OnTowerPlaced.AddDynamic(this, &AEnemyHandler::NotifyGridChanged);
            BuildManager->OnTowerDeleted.AddDynamic(this, &AEnemyHandler::NotifyGridChanged);

            GetWorldTimerManager().ClearTimer(TryFindBuildManagerHandle);
        }
    }
}

/**
 * @brief Registers an enemy with the handler.
 *
 * Prevents duplicates and stores the reference as a TWeakObjectPtr.
 *
 * @param Enemy Enemy instance to register.
 */
void AEnemyHandler::RegisterEnemy(AEnemyBase* Enemy)
{
    if (!Enemy) return;

    for (auto& W : RegisteredEnemies)
    {
        if (W.IsValid() && W.Get() == Enemy)
            return;
    }

    RegisteredEnemies.Add(Enemy);
}

/**
 * @brief Unregisters an enemy when destroyed or removed.
 *
 * Cleans up invalid weak pointers as well.
 *
 * @param Enemy Enemy instance to unregister.
 */
void AEnemyHandler::UnregisterEnemy(AEnemyBase* Enemy)
{
    if (!Enemy) return;

    for (int32 i = RegisteredEnemies.Num() - 1; i >= 0; --i)
    {
        if (!RegisteredEnemies[i].IsValid() || RegisteredEnemies[i].Get() == Enemy)
        {
            RegisteredEnemies.RemoveAt(i);
        }
    }
}

/**
 * @brief Called whenever the grid changes (tower placed/removed).
 *
 * Iterates through all registered enemies and instructs them to recalculate paths.
 *
 * @param loc Tile coordinate affected by the grid change.
 * @param cost New movement cost of the tile.
 */
void AEnemyHandler::NotifyGridChanged(FVector2D loc, int32 cost)
{
    for (int32 i = RegisteredEnemies.Num() - 1; i >= 0; --i)
    {
        if (RegisteredEnemies[i].IsValid())
        {
            AEnemyBase* E = RegisteredEnemies[i].Get();
            if (E)
            {
                E->RecalculatePath();
            }
        }
        else
        {
            RegisteredEnemies.RemoveAt(i);
        }
    }
}

/**
 * @brief Determines whether a tile is walkable based on its occupancy.
 *
 * @param X Grid X coordinate.
 * @param Y Grid Y coordinate.
 * @return true if walkable (Empty/Spawn/Goal), false otherwise.
 */
bool AEnemyHandler::IsTileWalkable(int32 X, int32 Y) const
{
    if (!GridManager) return false;
    if (!GridManager->IsValidTile(X, Y)) return false;

    FTileData Tile;
    if (!GridManager->GetTileSafe(X, Y, Tile)) return false;

    switch (Tile.Occupancy)
    {
    case ETileOccupancyState::Empty:
    case ETileOccupancyState::Spawn:
    case ETileOccupancyState::Goal:
        return true;

    default:
        return false;
    }
}

/**
 * @brief Returns the four cardinal (N/E/S/W) neighbors of a tile.
 *
 * @param P Tile coordinate.
 * @return Array of neighboring tile coordinates.
 */
TArray<FIntPoint> AEnemyHandler::GetNeighbors4(const FIntPoint& P) const
{
    TArray<FIntPoint> N;
    N.Reserve(4);
    N.Add(FIntPoint(P.X + 1, P.Y));
    N.Add(FIntPoint(P.X - 1, P.Y));
    N.Add(FIntPoint(P.X, P.Y + 1));
    N.Add(FIntPoint(P.X, P.Y - 1));
    return N;
}

/**
 * @brief Performs A* pathfinding on the tile grid.
 *
 * Converts world positions into tile coordinates, performs A*, and
 * outputs an array of world locations representing the path.
 *
 * @param StartWorld Starting world location.
 * @param EndWorld Target world location.
 * @param OutPath Filled with the resulting path if successful.
 * @return true if a valid path to the target exists, false otherwise.
 */
bool AEnemyHandler::FindPath(const FVector& StartWorld, const FVector& EndWorld, TArray<FVector>& OutPath)
{
    OutPath.Empty();

    if (!GridManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyHandler::FindPath - GridManager missing."));
        return false;
    }

    // Convert world → tile coordinates
    FVector2D StartGridF, EndGridF;
    if (!GridManager->WorldToGrid(StartWorld, StartGridF))
    {
        UE_LOG(LogTemp, Log,
            TEXT("EnemyHandler::FindPath - start position outside grid: %s"),
            *StartWorld.ToString());
        return false;
    }

    if (!GridManager->WorldToGrid(EndWorld, EndGridF))
    {
        UE_LOG(LogTemp, Log,
            TEXT("EnemyHandler::FindPath - end position outside grid: %s"),
            *EndWorld.ToString());
        return false;
    }

    const FIntPoint StartGrid((int32)StartGridF.X, (int32)StartGridF.Y);
    const FIntPoint GoalGrid((int32)EndGridF.X, (int32)EndGridF.Y);

    // Early rejection: goal must be walkable
    if (!IsTileWalkable(GoalGrid.X, GoalGrid.Y))
    {
        UE_LOG(LogTemp, Log,
            TEXT("EnemyHandler::FindPath - goal not walkable (%d,%d)"),
            GoalGrid.X, GoalGrid.Y);
        return false;
    }

    // A* data structures
    TArray<FPathNode> Nodes;
    Nodes.Reserve(1024);
    TMap<FIntPoint, int32> NodeIndexMap;
    TArray<int32> OpenList;
    TSet<FIntPoint> ClosedSet;

    auto AddNode = [&](const FIntPoint& Coord, int32 ParentIdx, float G, float H) -> int32
        {
            FPathNode Node;
            Node.Coord = Coord;
            Node.ParentIndex = ParentIdx;
            Node.G = G;
            Node.H = H;

            int32 Idx = Nodes.Add(MoveTemp(Node));
            NodeIndexMap.Add(Coord, Idx);
            return Idx;
        };

    auto Heuristic = [&](const FIntPoint& A, const FIntPoint& B) -> float
        {
            return FVector2D::Distance(FVector2D(A.X, A.Y), FVector2D(B.X, B.Y));
        };

    // Initialize A*
    int32 StartIdx = AddNode(StartGrid, -1, 0.f, Heuristic(StartGrid, GoalGrid));
    OpenList.Add(StartIdx);
    int32 FoundGoalIdx = INDEX_NONE;

    // A* loop
    while (OpenList.Num() > 0)
    {
        // Select node with smallest F score
        int32 BestPos = 0;
        int32 BestNodeIdx = OpenList[0];
        float BestF = Nodes[BestNodeIdx].F();

        for (int32 i = 1; i < OpenList.Num(); ++i)
        {
            int32 idx = OpenList[i];
            float f = Nodes[idx].F();
            if (f < BestF)
            {
                BestF = f;
                BestNodeIdx = idx;
                BestPos = i;
            }
        }

        OpenList.RemoveAt(BestPos);
        ClosedSet.Add(Nodes[BestNodeIdx].Coord);

        // Goal reached
        if (Nodes[BestNodeIdx].Coord == GoalGrid)
        {
            FoundGoalIdx = BestNodeIdx;
            break;
        }

        // Expand neighbors
        for (const FIntPoint& NCoord : GetNeighbors4(Nodes[BestNodeIdx].Coord))
        {
            if (!GridManager->IsValidTile(NCoord.X, NCoord.Y)) continue;
            if (!IsTileWalkable(NCoord.X, NCoord.Y)) continue;
            if (ClosedSet.Contains(NCoord)) continue;

            float TentativeG = Nodes[BestNodeIdx].G + 1.0f;

            int32* Existing = NodeIndexMap.Find(NCoord);
            if (Existing)
            {
                int32 existingIdx = *Existing;
                if (TentativeG < Nodes[existingIdx].G)
                {
                    Nodes[existingIdx].G = TentativeG;
                    Nodes[existingIdx].ParentIndex = BestNodeIdx;
                }

                if (!OpenList.Contains(existingIdx))
                    OpenList.Add(existingIdx);
            }
            else
            {
                float H = Heuristic(NCoord, GoalGrid);
                int32 NewIdx = AddNode(NCoord, BestNodeIdx, TentativeG, H);
                OpenList.Add(NewIdx);
            }
        }
    }

    if (FoundGoalIdx == INDEX_NONE)
    {
        return false;
    }

    // Reconstruct world-space path
    TArray<FVector> ReversePath;
    int32 Cursor = FoundGoalIdx;

    while (Cursor != -1 && Nodes.IsValidIndex(Cursor))
    {
        const FIntPoint& C = Nodes[Cursor].Coord;

        FTileData Tile;
        if (GridManager->GetTileSafe(C.X, C.Y, Tile))
        {
            ReversePath.Add(Tile.WorldLocation);
        }

        Cursor = Nodes[Cursor].ParentIndex;
    }

    Algo::Reverse(ReversePath);
    OutPath = MoveTemp(ReversePath);
    return true;
}
