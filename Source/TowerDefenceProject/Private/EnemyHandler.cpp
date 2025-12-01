#include "EnemyHandler.h"
#include "Kismet/GameplayStatics.h"
#include "Grid/GridManager.h"
#include "EnemyBase.h"
#include "Algo/Reverse.h"

AEnemyHandler::AEnemyHandler()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemyHandler::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(TryFindGridManagerHandle, this, &AEnemyHandler::TryFindGridManager, 0.1f, true);
}

void AEnemyHandler::TryFindGridManager()
{
    if (GridManager == nullptr)
    {
        GridManager = Cast<AGridManager>(
            UGameplayStatics::GetActorOfClass(this, AGridManager::StaticClass())
        );

        if (GridManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: Found GridManager after spawn."));

            GetWorldTimerManager().ClearTimer(TryFindGridManagerHandle);
        }
    }
}

void AEnemyHandler::RegisterEnemy(AEnemyBase* Enemy)
{
    if (!Enemy) return;
    for (auto& W : RegisteredEnemies) if (W.IsValid() && W.Get() == Enemy) return;
    RegisteredEnemies.Add(Enemy);
}

void AEnemyHandler::UnregisterEnemy(AEnemyBase* Enemy)
{
    if (!Enemy) return;
    for (int32 i = RegisteredEnemies.Num() - 1; i >= 0; --i)
    {
        if (!RegisteredEnemies[i].IsValid() || RegisteredEnemies[i].Get() == Enemy)
            RegisteredEnemies.RemoveAt(i);
    }
}

void AEnemyHandler::NotifyGridChanged()
{
    for (int32 i = RegisteredEnemies.Num() - 1; i >= 0; --i)
    {
        if (RegisteredEnemies[i].IsValid())
        {
            AEnemyBase* E = RegisteredEnemies[i].Get();
            if (E) E->RecalculatePath();
        }
        else
        {
            RegisteredEnemies.RemoveAt(i);
        }
    }
}

bool AEnemyHandler::IsTileWalkable(int32 X, int32 Y) const
{
    if (!GridManager) return false;
    if (!GridManager->IsValidTile(X, Y)) return false;

    FTileData Tile;
    if (!GridManager->GetTileSafe(X, Y, Tile)) return false;

    // Consider spawn & goal tiles walkable as well as empty
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

bool AEnemyHandler::FindPath(const FVector& StartWorld, const FVector& EndWorld, TArray<FVector>& OutPath)
{
    OutPath.Empty();

    if (!GridManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyHandler::FindPath - GridManager missing."));
        return false;
    }

    // Convert world -> grid coordinates (tile indices)
    FVector2D StartGridF, EndGridF;
    if (!GridManager->WorldToGrid(StartWorld, StartGridF))
    {
        UE_LOG(LogTemp, Log, TEXT("EnemyHandler::FindPath - start position outside grid: %s"), *StartWorld.ToString());
        return false;
    }
    if (!GridManager->WorldToGrid(EndWorld, EndGridF))
    {
        UE_LOG(LogTemp, Log, TEXT("EnemyHandler::FindPath - end position outside grid: %s"), *EndWorld.ToString());
        return false;
    }

    const FIntPoint StartGrid((int32)StartGridF.X, (int32)StartGridF.Y);
    const FIntPoint GoalGrid((int32)EndGridF.X, (int32)EndGridF.Y);

    // Quick goal walkable check
    if (!IsTileWalkable(GoalGrid.X, GoalGrid.Y))
    {
        UE_LOG(LogTemp, Log, TEXT("EnemyHandler::FindPath - goal tile not walkable (%d,%d)"), GoalGrid.X, GoalGrid.Y);
        return false;
    }

    // A* structures
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
            // Manhattan or Euclidean; use Euclidean for now
            return FVector2D::Distance(FVector2D(A.X, A.Y), FVector2D(B.X, B.Y));
        };

    // Start node
    int32 StartIdx = AddNode(StartGrid, -1, 0.f, Heuristic(StartGrid, GoalGrid));
    OpenList.Add(StartIdx);
    int32 FoundGoalIdx = INDEX_NONE;

    while (OpenList.Num() > 0)
    {
        // pick open node with smallest F
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

        // pop best
        OpenList.RemoveAt(BestPos);
        ClosedSet.Add(Nodes[BestNodeIdx].Coord);

        // check goal
        if (Nodes[BestNodeIdx].Coord == GoalGrid)
        {
            FoundGoalIdx = BestNodeIdx;
            break;
        }

        // neighbors
        for (const FIntPoint& NCoord : GetNeighbors4(Nodes[BestNodeIdx].Coord))
        {
            if (!GridManager->IsValidTile(NCoord.X, NCoord.Y)) continue;
            if (!IsTileWalkable(NCoord.X, NCoord.Y)) continue;
            if (ClosedSet.Contains(NCoord)) continue;

            float TentativeG = Nodes[BestNodeIdx].G + 1.0f; // cost between neighbors = 1

            int32* Existing = NodeIndexMap.Find(NCoord);
            if (Existing)
            {
                int32 existingIdx = *Existing;
                if (TentativeG < Nodes[existingIdx].G)
                {
                    Nodes[existingIdx].G = TentativeG;
                    Nodes[existingIdx].ParentIndex = BestNodeIdx;
                }
                if (!OpenList.Contains(existingIdx)) OpenList.Add(existingIdx);
            }
            else
            {
                float H = Heuristic(NCoord, GoalGrid);
                int32 newIdx = AddNode(NCoord, BestNodeIdx, TentativeG, H);
                OpenList.Add(newIdx);
            }
        }
    } // end while

    if (FoundGoalIdx == INDEX_NONE)
    {
        // No path found
        return false;
    }

    // Reconstruct path: tile centers from goal to start
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
