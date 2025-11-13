#include "EnemyHandler.h"
#include "Kismet/GameplayStatics.h"
#include "Grid/GridManager.h"
#include "EnemyBase.h"
#include "Containers/Queue.h"
#include "Algo/Reverse.h"

AEnemyHandler::AEnemyHandler()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemyHandler::BeginPlay()
{
    Super::BeginPlay();

    if (!GridManager)
    {
        GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
        if (!GridManager)
        {
            UE_LOG(LogTemp, Error, TEXT("EnemyHandler: No GridManager found in world; pathfinding will fail until one is assigned."));
        }
    }
}

void AEnemyHandler::RegisterEnemy(AEnemyBase* Enemy)
{
    if (!Enemy) return;

    // avoid duplicates
    for (auto& Weak : RegisteredEnemies)
    {
        if (Weak.IsValid() && Weak.Get() == Enemy)
            return;
    }

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
    // Ask each registered enemy to recalc path
    for (int32 i = RegisteredEnemies.Num() - 1; i >= 0; --i)
    {
        if (RegisteredEnemies[i].IsValid())
        {
            AEnemyBase* Enemy = RegisteredEnemies[i].Get();
            if (Enemy)
            {
                Enemy->RecalculatePath();
            }
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
    if (!GridManager->TileGrid.IsValidIndex(X) || !GridManager->TileGrid[X].IsValidIndex(Y)) return false;

    const FTileData& Tile = GridManager->TileGrid[X][Y];
    return Tile.Occupancy == ETileOccupancyState::Empty;
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

    FVector2D StartGridF, EndGridF;
    if (!GridManager->WorldToGrid(StartWorld, StartGridF) || !GridManager->WorldToGrid(EndWorld, EndGridF))
    {
        UE_LOG(LogTemp, Log, TEXT("world position outside grid"));
        return false;
    }

    const FIntPoint StartGrid((int32)StartGridF.X, (int32)StartGridF.Y);
    const FIntPoint GoalGrid((int32)EndGridF.X, (int32)EndGridF.Y);

    // Quick check: goal should be walkable
    if (!IsTileWalkable(GoalGrid.X, GoalGrid.Y))
    {
        return false;
    }

    // A* implementation using arrays + map for node lookup
    TArray<FPathNode> Nodes;
    Nodes.Reserve(512); // heuristic reserve

    // Map from coord -> index in Nodes array
    TMap<FIntPoint, int32> NodeIndexMap;
    auto AddNode = [&](const FIntPoint& Coord, int32 ParentIdx, float G, float H) -> int32
        {
            FPathNode Node;
            Node.Coord = Coord;
            Node.ParentIndex = ParentIdx;
            Node.G = G;
            Node.H = H;
            int32 NewIndex = Nodes.Add(MoveTemp(Node));
            NodeIndexMap.Add(Coord, NewIndex);
            return NewIndex;
        };

    // Open list stores indices into Nodes
    TArray<int32> OpenList;
    TSet<FIntPoint> ClosedSet;

    auto Heuristic = [&](const FIntPoint& A, const FIntPoint& B) -> float
        {
            // Use Manhattan or Euclidean; grid distance works fine
            return FVector2D::Distance(FVector2D(A.X, A.Y), FVector2D(B.X, B.Y));
        };

    // Create start node
    int32 StartIdx = AddNode(StartGrid, -1, 0.f, Heuristic(StartGrid, GoalGrid));
    OpenList.Add(StartIdx);

    int32 FoundGoalNodeIdx = INDEX_NONE;

    while (OpenList.Num() > 0)
    {
        // find node in open list with smallest F
        int32 BestOpenIdx = OpenList[0];
        float BestF = Nodes[BestOpenIdx].F();
        int32 BestOpenListPos = 0;

        for (int32 i = 1; i < OpenList.Num(); ++i)
        {
            int32 NodeIdx = OpenList[i];
            float NodeF = Nodes[NodeIdx].F();
            if (NodeF < BestF)
            {
                BestF = NodeF;
                BestOpenIdx = NodeIdx;
                BestOpenListPos = i;
            }
        }

        // remove from open list
        OpenList.RemoveAt(BestOpenListPos);
        ClosedSet.Add(Nodes[BestOpenIdx].Coord);

        // if goal
        if (Nodes[BestOpenIdx].Coord == GoalGrid)
        {
            FoundGoalNodeIdx = BestOpenIdx;
            break;
        }

        // neighbors
        for (const FIntPoint& NCoord : GetNeighbors4(Nodes[BestOpenIdx].Coord))
        {
            // bounds & walkable check
            if (!GridManager->TileGrid.IsValidIndex(NCoord.X) || !GridManager->TileGrid[NCoord.X].IsValidIndex(NCoord.Y))
                continue;
            if (!IsTileWalkable(NCoord.X, NCoord.Y))
                continue;
            if (ClosedSet.Contains(NCoord))
                continue;

            float TentativeG = Nodes[BestOpenIdx].G + 1.0f; // cost between adjacent tiles = 1; adjust if using weighted costs

            int32* ExistingPtr = NodeIndexMap.Find(NCoord);
            if (ExistingPtr)
            {
                int32 ExistingIdx = *ExistingPtr;
                if (TentativeG < Nodes[ExistingIdx].G)
                {
                    Nodes[ExistingIdx].G = TentativeG;
                    Nodes[ExistingIdx].ParentIndex = BestOpenIdx;
                }
                // already in open list (we don't need to re-add)
                if (!OpenList.Contains(ExistingIdx))
                    OpenList.Add(ExistingIdx);
            }
            else
            {
                float H = Heuristic(NCoord, GoalGrid);
                int32 NewIdx = AddNode(NCoord, BestOpenIdx, TentativeG, H);
                OpenList.Add(NewIdx);
            }
        }
    } // end while

    if (FoundGoalNodeIdx == INDEX_NONE)
    {
        // No path found
        return false;
    }

    // build reverse path (tile centers)
    TArray<FVector> ReversePath;
    int32 Cursor = FoundGoalNodeIdx;
    while (Cursor != -1 && Nodes.IsValidIndex(Cursor))
    {
        const FIntPoint& C = Nodes[Cursor].Coord;
        const FTileData& Tile = GridManager->TileGrid[C.X][C.Y];
        ReversePath.Add(Tile.WorldLocation);
        Cursor = Nodes[Cursor].ParentIndex;
    }

    Algo::Reverse(ReversePath);
    OutPath = MoveTemp(ReversePath);
    return true;
}