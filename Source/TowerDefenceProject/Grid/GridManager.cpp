
#include "Grid/GridManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	
	TileGrid.Empty();
	InitializeGrid();
}


/*--- Public API ---*/
void AGridManager::InitializeGrid()
{
	TileGrid.Empty();
	TileMeshes.Empty();

	const int32 TotalTiles = GridSizeX * GridSizeY;
	TileGrid.AddZeroed(TotalTiles); // Add at the end

	for (int32 Y = 0; Y < GridSizeY; ++Y)
	{
		for (int32 X = 0; X < GridSizeX; ++X)
		{
			const int32 Index = GetTileIndex(X, Y);
			FTileData& Tile = TileGrid[Index];

			Tile.GridLocation = FVector2D(X, Y);
			Tile.WorldLocation = GetTileWorldLocation(X, Y);
			Tile.Occupancy = ETileOccupancyState::Empty;
			Tile.InternalVisualState = static_cast<uint8>(ETileVisualState::Default);

			CreateTileMesh(X, Y);
			UpdateTileVisualInternal(X, Y);
		}
	}
	// Generate course
	GenerateCourse();

	UE_LOG(LogTemp, Log, TEXT("GridManager: Initialized %dx%d grid"), GridSizeX, GridSizeY);
}

bool AGridManager::CanPlaceTowerAt(FIntPoint Tile)
{
	// Validate tile indices
	if (!IsValidTile(Tile.X, Tile.Y))
		return false;

	const int32 Index = GetTileIndex(Tile.X, Tile.Y);
	FTileData& TileRef = TileGrid[Index];
	
	if (TileRef.Occupancy != ETileOccupancyState::Empty)
		return false;

	// Temporarily mark as blocked (or Tower) so pathfinder treats it as non-walkable
	TileRef.Occupancy = ETileOccupancyState::Blocked;
	TileRef.OccupantActor = nullptr;

	const int32 SpawnX = FMath::RoundToInt(SpawnTile.X);
	const int32 SpawnY = FMath::RoundToInt(SpawnTile.Y);
	const int32 GoalX = FMath::RoundToInt(GoalTile.X);
	const int32 GoalY = FMath::RoundToInt(GoalTile.Y);

	// Convert spawn/goal tile indices to world locations (tile centers)
	const FVector SpawnWorld = GetTileWorldLocation(SpawnX, SpawnY);
	const FVector GoalWorld = GetTileWorldLocation(GoalX, GoalY);

	// Ask the enemy handler to find a path with the tile temporarily blocked
	TArray<FVector> TempPath;
	const bool bPathExists = EnemyHandler->FindPath(SpawnWorld, GoalWorld, TempPath);

	// Revert tile occupancy to original state
	TileRef.Occupancy = ETileOccupancyState::Empty;

	return bPathExists;
}

bool AGridManager::IsValidTile(int32 X, int32 Y) const
{
	return X >= 0 && X < GridSizeX && Y >= 0 && Y < GridSizeY;
}

bool AGridManager::GetTileSafe(int32 X, int32 Y, FTileData& OutTile) const
{
	if (!IsValidTile(X, Y)) return false;
	OutTile = TileGrid[GetTileIndex(X, Y)];
	return true;
}

FVector AGridManager::GetTileWorldLocation(int32 X, int32 Y) const
{
	const FVector Offset = FVector(TileSize * 0.5f, TileSize * 0.5f, 0.f);
	return GetActorLocation() + FVector(X * TileSize, Y * TileSize, 0.f) + Offset;
}

bool AGridManager::WorldToGrid(const FVector& WorldLocation, FVector2D& OutGrid) const
{
	const FVector Local = WorldLocation - GetActorLocation();
	const int32 X = FMath::FloorToInt(Local.X / TileSize);
	const int32 Y = FMath::FloorToInt(Local.Y / TileSize);
	if (!IsValidTile(X, Y)) return false;

	OutGrid = FVector2D(X, Y);
	return true;
}

bool AGridManager::SetTileOccupant(int32 X, int32 Y, AActor* NewOccupant)
{
	if (!IsValidTile(X, Y)) return false;

	const int32 Index = GetTileIndex(X, Y);
	FTileData& Tile = TileGrid[Index];

	Tile.OccupantActor = NewOccupant;
	Tile.Occupancy = NewOccupant ? ETileOccupancyState::Tower : ETileOccupancyState::Empty;

	UpdateTileVisualInternal(X, Y);
	OnTileOccupancyChanged.Broadcast(X, Y);

	return true;
}

void AGridManager::SetTileVisual(int32 X, int32 Y, ETileVisualState NewState)
{
	if (!IsValidTile(X, Y)) return;

	const int32 Index = GetTileIndex(X, Y);
	FTileData& Tile = TileGrid[Index];

	const uint8 NewIdx = static_cast<uint8>(NewState);
	if (Tile.InternalVisualState == NewIdx) return;

	Tile.InternalVisualState = NewIdx;
	UpdateTileVisualInternal(X, Y);
	OnTileVisualChanged.Broadcast(X, Y);
}

bool AGridManager::SetTileOccupancy(int32 X, int32 Y, ETileOccupancyState NewOccupancy, AActor* NewOccupant)
{
	if (!IsValidTile(X, Y)) return false;

	const int32 Index = GetTileIndex(X, Y);
	FTileData& Tile = TileGrid[Index];

	Tile.Occupancy = NewOccupancy;
	Tile.OccupantActor = NewOccupant;

	return true;
}


/*--- Internal Helpers ---*/
int32 AGridManager::GetTileIndex(int32 X, int32 Y) const
{
	return X + Y * GridSizeX;
}

void AGridManager::CreateTileMesh(int32 X, int32 Y)
{
	const int32 Index = GetTileIndex(X, Y);
	if (TileMeshes.Contains(Index)) return;

	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this,
		*FString::Printf(TEXT("TileMesh_%d_%d"), X, Y));
	Mesh->RegisterComponent();

	USceneComponent* AttachTarget = RootComponent ?
		static_cast<USceneComponent*>(RootComponent) :
		static_cast<USceneComponent*>(GetRootComponent());

	Mesh->AttachToComponent(AttachTarget,
		FAttachmentTransformRules::KeepRelativeTransform);

	Mesh->SetRelativeLocation(GetTileWorldLocation(X, Y) - GetActorLocation());

	if (TileMeshAsset)
		Mesh->SetStaticMesh(TileMeshAsset);
	else
		UE_LOG(LogTemp, Warning, TEXT("GridManager: No Tile Mesh!"));

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	TileMeshes.Add(Index, Mesh);
}

void AGridManager::UpdateTileVisualInternal(int32 X, int32 Y)
{
	const int32 Index = GetTileIndex(X, Y);
	const FTileData& Tile = TileGrid[Index];

	// Use what is stored
	ETileVisualState State = static_cast<ETileVisualState>(Tile.InternalVisualState);

	if (UStaticMeshComponent* Mesh = TileMeshes.FindRef(Index))
	{
		if (UMaterialInterface* Mat = VisualMaterials.FindRef(State))
		{
			Mesh->SetMaterial(0, Mat);
		}
	}

	// Old code - might be useful
	//const ETileVisualState Visual = GetVisualStateForOccupancy(Tile.Occupancy);
	//const uint8 VisualIdx = static_cast<uint8>(Visual);
	//TileGrid[Index].InternalVisualState = VisualIdx; // Keep cache in sync

	//UStaticMeshComponent* Mesh = TileMeshes.FindRef(Index);
	//if (!Mesh) return;

	//UMaterialInterface* const* MatPtr = VisualMaterials.Find(Visual);
	//if (MatPtr) Mesh->SetMaterial(0, *MatPtr);
}

void AGridManager::GenerateCourse()
{
	// Collect every Tile on the edge
	TArray<FVector2D> EdgeTiles;
	for (int32 X = 0; X < GridSizeX; X++) // Top + Bott
	{
		EdgeTiles.Add(FVector2D(X, 0));
		EdgeTiles.Add(FVector2D(X, GridSizeY - 1));
	}
	for (int32 Y = 1; Y < GridSizeY - 1; Y++) // Right + Left
	{
		EdgeTiles.Add(FVector2D(0, Y));
		EdgeTiles.Add(FVector2D(GridSizeX - 1, Y));
	}

	// Pick two different tiles - not adjecent
	FVector2D Spawn, Goal;
	int32 Attempts = 0;
	const int32 MaxAttempts = 100;

	do
	{
		const int32 A = FMath::RandRange(0, EdgeTiles.Num() - 1);
		const int32 B = FMath::RandRange(0, EdgeTiles.Num() - 1);

		Spawn = EdgeTiles[A];
		Goal = EdgeTiles[B];

		if (++Attempts > MaxAttempts) break; // safety
	}
	while (Spawn == Goal || FVector2D::Distance(Spawn, Goal) <= 1.5f);

	// Apply
	SetTileOccupancy(Spawn.X, Spawn.Y, ETileOccupancyState::Spawn);
	SetTileOccupancy(Goal.X, Goal.Y, ETileOccupancyState::Goal);

	SpawnTile = Spawn;
	GoalTile = Goal;

	// Update visuals
	SetTileVisual(Spawn.X, Spawn.Y, ETileVisualState::Spawn);
	SetTileVisual(Goal.X, Goal.Y, ETileVisualState::Goal);

	UE_LOG(LogTemp, Log, TEXT("Random course: Spawn(%d,%d) Goal(%d,%d)"),
		(int32)Spawn.X, (int32)Spawn.Y, (int32)Goal.X, (int32)Goal.Y);
}

//ETileVisualState AGridManager::GetVisualStateForOccupancy(ETileOccupancyState Occupancy) const
//{
//	switch (Occupancy)
//	{
//	case ETileOccupancyState::Empty:	return ETileVisualState::Buildable;
//	case ETileOccupancyState::Tower:	return ETileVisualState::Occupied;
//	case ETileOccupancyState::Path:	return ETileVisualState::Path;
//	case ETileOccupancyState::Blocked:	return ETileVisualState::Blocked;
//	case ETileOccupancyState::Spawn:	return ETileVisualState::Spawn;
//	case ETileOccupancyState::Goal:	return ETileVisualState::Goal;
//	default:							return ETileVisualState::Default;
//	}
//}

/*--- Debug Commands ---*/
void AGridManager::DebugPrintGrid() const
{
	UE_LOG(LogTemp, Log, TEXT("=== GRID STATE ==="));
	for (int32 Y = 0; Y < GridSizeY; ++Y)
	{
		FString Row;
		for (int32 X = 0; X < GridSizeX; ++X)
		{
			const FTileData& T = TileGrid[GetTileIndex(X, Y)];
			Row += (T.Occupancy == ETileOccupancyState::Tower) ? TEXT("T") : TEXT(".");
		}
		UE_LOG(LogTemp, Log, TEXT("%s"), *Row);
	}
}

void AGridManager::DebugSetVisual(int32 X, int32 Y, int32 State)
{
	if (IsValidTile(X, Y))
	{
		SetTileVisual(X, Y, static_cast<ETileVisualState>(State));
	}
}