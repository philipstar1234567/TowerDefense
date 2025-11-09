
#include "Grid/GridManager.h"

// Constructor
AGridManager::AGridManager()
{
	// Initialize Components
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot; // Set as root

	TileMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(RootComponent);
	TileMesh->NumCustomDataFloats = 4; // RGBA | color change logic
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Variables
	TileSize = 100.f;
	GridWidth = 10;
	GridHeight = 10;

	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	
	TileGrid.Empty();
	
	GenerateGrid();
	SpawnTileVisuals();
}


void AGridManager::GenerateGrid()
{
	TileGrid.SetNum(GridWidth);
	for (int32 X = 0; X < GridWidth; ++X)
	{
		TileGrid[X].SetNum(GridHeight);

		for (int32 Y = 0; Y < GridHeight; ++Y)
		{
			FVector WorldLocation = FVector(X * TileSize + TileSize * 0.5f, Y * TileSize + TileSize * 0.5f, 0.f);
			FVector2D GridLocation = FVector2D(X, Y);

			FTileData NewTile;
			NewTile.WorldLocation = WorldLocation;
			NewTile.GridLocation = GridLocation;
			NewTile.InstanceIndex = -1;
			NewTile.Occupancy = ETileOccupancyState::Empty; // regarding gameplay logic for building
			NewTile.VisualState = ETileVisualState::Default; // regarding color for visuals

			TileGrid[X][Y] = NewTile;
		}
	}
}

void AGridManager::SpawnTileVisuals()
{
	if (!TileMesh) return;

	TileMesh->ClearInstances(); // Clear old instances if any

	for (int32 X = 0; X < GridWidth; ++X)
	{
		for(int32 Y = 0; Y < GridHeight; ++Y)
		{
			FTileData& Tile = TileGrid[X][Y];
			FTransform TileTransform(FRotator::ZeroRotator, Tile.WorldLocation, FVector(1.f, 1.f, 0.1f));

			int32 InstanceIndex = TileMesh->AddInstance(TileTransform);
			Tile.InstanceIndex = InstanceIndex; // Save to TileData

			UpdateTileVisual(Tile);// SetInitialColor
		}
	}
}

bool AGridManager::SetTileVisual(int32 X, int32 Y, ETileVisualState NewVisualState)
{
	if (!TileGrid.IsValidIndex(X) || !TileGrid[X].IsValidIndex(Y))
		return false;

	FTileData& Tile = TileGrid[X][Y];
	Tile.VisualState = NewVisualState;

	UpdateTileVisual(Tile); // Apply color
	return true;
}

bool AGridManager::SetTileOccupancy(int32 X, int32 Y, ETileOccupancyState NewOccupancy)
{
	if (!TileGrid.IsValidIndex(X) || !TileGrid[X].IsValidIndex(Y))
		return false;

	TileGrid[X][Y].Occupancy = NewOccupancy;
	return true;
}

bool AGridManager::WorldToGrid(const FVector& WorldLocation, FVector2D& OutGridCoords) const
{
	FVector GridOrigin = GetActorLocation();

	// Adjust for tile centering
	float LocalX = WorldLocation.X - GridOrigin.X;
	float LocalY = WorldLocation.Y - GridOrigin.Y;

	int32 GridX = FMath::FloorToInt(LocalX / TileSize);
	int32 GridY = FMath::FloorToInt(LocalY / TileSize);

	// Bounds check
	if (GridX < 0 || GridX >= GridWidth || GridY < 0 || GridY >= GridHeight)
	{
		OutGridCoords = FVector2D(-1, -1);
		return false;
	}

	OutGridCoords = FVector2D(GridX, GridY);
	return true;
}

bool AGridManager::GetTileSafe(int32 X, int32 Y, FTileData& OutTile) const
{
	if (TileGrid.IsValidIndex(X) && TileGrid[X].IsValidIndex(Y))
	{
		OutTile = TileGrid[X][Y];
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("GridManager: Invaldig tile access at X=%d, Y=%d"), X, Y);
	return false;
}

void AGridManager::UpdateTileVisual(const FTileData& Tile)
{
	if (!TileMesh || Tile.InstanceIndex == -1) return;

	FVector4 Color;
	switch (Tile.VisualState)
	{
		case ETileVisualState::Buildable:
			Color = FVector4(0.f, 1.f, 0.f, 1.f); // Green
			break;
		case ETileVisualState::Blocked:
			Color = FVector4(0.5f, 0.5f, 0.5f, 1.f); // Grey
			break;
		case ETileVisualState::Occupied:
			Color = FVector4(1.f, 0.f, 0.f, 1.f); // Red
			break;
		case ETileVisualState::Highlighted:
			Color = FVector4(1.f, 0.6f, 0.f, 1.f); // Yellow
		default:
			Color = FVector4(1.f, 1.f, 1.f, 1.f); // White
			break; 
	}

	TileMesh->SetCustomDataValue(Tile.InstanceIndex, 0, Color.X); // R
	TileMesh->SetCustomDataValue(Tile.InstanceIndex, 1, Color.Y); // G
	TileMesh->SetCustomDataValue(Tile.InstanceIndex, 2, Color.Z); // B
	TileMesh->SetCustomDataValue(Tile.InstanceIndex, 3, Color.W); // A

	TileMesh->MarkRenderStateDirty(); // Refresh
}

bool AGridManager::FindPath(const FVector& StartWorld, const FVector& EndWorld, TArray<FVector>& OutPath)
{
	OutPath.Empty();

	FVector2D StartGrid, EndGrid;
	if (!WorldToGrid(StartWorld, StartGrid) || !WorldToGrid(EndWorld, EndGrid))
		return false;

	const int32 StartX = StartGrid.X;
	const int32 StartY = StartGrid.Y;
	const int32 EndX = EndGrid.X;
	const int32 EndY = EndGrid.Y;

	if (!TileGrid.IsValidIndex(StartX) || !TileGrid[StartX].IsValidIndex(StartY) ||
		!TileGrid.IsValidIndex(EndX) || !TileGrid[EndX].IsValidIndex(EndY))
		return false;

	// If goal is blocked, fail
	if (TileGrid[EndX][EndY].Occupancy != ETileOccupancyState::Empty)
		return false;

	// Open and closed lists
	TArray<FPathNode*> OpenList;
	TArray<FPathNode*> ClosedList;

	auto Heuristic = [&](int32 X, int32 Y)
		{
			return FVector2D::Distance(FVector2D(X, Y), FVector2D(EndX, EndY));
		};

	// Starting node
	FPathNode* StartNode = new FPathNode(StartX, StartY, 0.f, Heuristic(StartX, StartY), nullptr);
	OpenList.Add(StartNode);

	FPathNode* EndNode = nullptr;

	// Directions (4-way)
	const TArray<FIntPoint> Directions = {
		{1,0}, {-1,0}, {0,1}, {0,-1}
	};

	while (OpenList.Num() > 0)
	{
		// Get node with lowest F cost
		OpenList.Sort([](const FPathNode& A, const FPathNode& B)
			{
				return A.GetFCost() < B.GetFCost();
			});

		FPathNode* Current = OpenList[0];
		OpenList.RemoveAt(0);
		ClosedList.Add(Current);

		// Check goal
		if (Current->X == EndX && Current->Y == EndY)
		{
			EndNode = Current;
			break;
		}

		// Explore neighbors
		for (const FIntPoint& Dir : Directions)
		{
			int32 NX = Current->X + Dir.X;
			int32 NY = Current->Y + Dir.Y;

			if (!TileGrid.IsValidIndex(NX) || !TileGrid[NX].IsValidIndex(NY))
				continue;

			const FTileData& NeighborTile = TileGrid[NX][NY];
			if (NeighborTile.Occupancy != ETileOccupancyState::Empty)
				continue;

			bool bInClosed = false;
			for (auto* N : ClosedList)
				if (N->X == NX && N->Y == NY)
					bInClosed = true;
			if (bInClosed)
				continue;

			float NewGCost = Current->GCost + FVector2D::Distance(
				FVector2D(Current->X, Current->Y),
				FVector2D(NX, NY)
			);

			FPathNode* ExistingOpenNode = nullptr;
			for (auto* N : OpenList)
				if (N->X == NX && N->Y == NY)
					ExistingOpenNode = N;

			if (ExistingOpenNode)
			{
				if (NewGCost < ExistingOpenNode->GCost)
				{
					ExistingOpenNode->GCost = NewGCost;
					ExistingOpenNode->Parent = Current;
				}
			}
			else
			{
				FPathNode* NewNode = new FPathNode(NX, NY, NewGCost, Heuristic(NX, NY), Current);
				OpenList.Add(NewNode);
			}
		}
	}

	// Build path if goal found
	if (EndNode)
	{
		TArray<FVector> ReversePath;
		FPathNode* Node = EndNode;
		while (Node)
		{
			const FTileData& Tile = TileGrid[Node->X][Node->Y];
			ReversePath.Add(Tile.WorldLocation);
			Node = Node->Parent;
		}

		Algo::Reverse(ReversePath);
		OutPath = ReversePath;
	}

	// Cleanup nodes
	for (auto* Node : OpenList) delete Node;
	for (auto* Node : ClosedList) delete Node;

	return (EndNode != nullptr);
}

