
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

void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
			NewTile.bIsOccupied = false;
			NewTile.TileState = ETileState::Default; // for color in build mode

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

bool AGridManager::SetTileState(int32 X, int32 Y, ETileState NewState)
{
	if (!TileGrid.IsValidIndex(X) || !TileGrid[X].IsValidIndex(Y))
		return false;

	FTileData& Tile = TileGrid[X][Y];
	Tile.TileState = NewState;

	UpdateTileVisual(Tile); // Update ISMC Color
	return true;
}

void AGridManager::UpdateTileVisual(const FTileData& Tile)
{
	if (!TileMesh || Tile.InstanceIndex == -1) return;

	FVector4 Color;
	switch (Tile.TileState)
	{
		case ETileState::Buildable: Color = FVector4(0.f, 1.f, 0.f, 1.f); break; // Green
		case ETileState::Occupied: Color = FVector4(1.f, 0.f, 0.f, 1.f); break; // Red
		default: Color = FVector4(1.f, 1.f, 1.f, 1.f); break; // White
	}

	TileMesh->SetCustomDataValue(Tile.InstanceIndex, 0, Color.X); // R
	TileMesh->SetCustomDataValue(Tile.InstanceIndex, 1, Color.Y); // G
	TileMesh->SetCustomDataValue(Tile.InstanceIndex, 2, Color.Z); // B
	TileMesh->SetCustomDataValue(Tile.InstanceIndex, 3, Color.W); // A

	TileMesh->MarkRenderStateDirty(); // Refresh
}

