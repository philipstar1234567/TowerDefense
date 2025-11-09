
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

