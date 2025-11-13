
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

	UE_LOG(LogTemp, Log, TEXT("GridManager: Initialized %dx%d grid"), GridSizeX, GridSizeY);
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

	// occupancy -> visual state
	const ETileVisualState Visual = GetVisualStateForOccupancy(Tile.Occupancy);
	const uint8 VisualIdx = static_cast<uint8>(Visual);
	TileGrid[Index].InternalVisualState = VisualIdx; // Keep cache in sync

	UStaticMeshComponent* Mesh = TileMeshes.FindRef(Index);
	if (!Mesh) return;

	UMaterialInterface* const* MatPtr = VisualMaterials.Find(Visual);
	if (MatPtr) Mesh->SetMaterial(0, *MatPtr);
}

ETileVisualState AGridManager::GetVisualStateForOccupancy(ETileOccupancyState Occupancy) const
{
	switch (Occupancy)
	{
	case ETileOccupancyState::Empty:	return ETileVisualState::Buildable;
	case ETileOccupancyState::Tower:	return ETileVisualState::Occupied;
	case ETileOccupancyState::Path:	return ETileVisualState::Path;
	case ETileOccupancyState::Blocked:	return ETileVisualState::Blocked;
	case ETileOccupancyState::Spawn:	return ETileVisualState::Spawn;
	case ETileOccupancyState::Goal:	return ETileVisualState::Goal;
	default:							return ETileVisualState::Default;
	}
}

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