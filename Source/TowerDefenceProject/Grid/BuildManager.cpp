
#include "Grid/BuildManager.h"

// Constructor
ABuildManager::ABuildManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABuildManager::SetBuildModeActive(bool bIsActive)
{
	// Enfore exclusivity
	if (bIsActive && bDeleteModeActive)
	{
		SetDeleteModeActive(false);
	}

	bBuildModeActive = bIsActive;

	if (bIsActive)
	{
		// Choose the selected tower class (fallback to default placeholder)
		if (!SelectedTowerClass)
			SelectedTowerClass = DefaultTowerClass;

		if (!TowerPreviewInstance && SelectedTowerClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			TowerPreviewInstance = GetWorld()->SpawnActor<AActor>(
				SelectedTowerClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

			if (TowerPreviewInstance)
			{
				TowerPreviewInstance->SetActorEnableCollision(false);
				TowerPreviewInstance->SetActorHiddenInGame(true);
				UE_LOG(LogTemp, Log, TEXT("BuildManager: Tower preview spawned."));
			}
		}
	}
	else
	{
		if (TowerPreviewInstance)
		{
			TowerPreviewInstance->Destroy();
			TowerPreviewInstance = nullptr;
			UE_LOG(LogTemp, Log, TEXT("BuildManager: Tower preview destroyed."));
		}
	}

	LastHoveredTile = FVector2D(-1, -1);
	UE_LOG(LogTemp, Log, TEXT("BuildManager: Build Mode set to %s"), bIsActive ?
		TEXT("Active") : TEXT("Inactive"));
}

void ABuildManager::SetDeleteModeActive(bool bIsActive)
{
	// Enforce exclusivity
	if (bIsActive && bBuildModeActive)
	{
		SetBuildModeActive(false); // destroy previuw
	}

	bDeleteModeActive = bIsActive;

	//reset hover
	LastHoveredTile = FVector2D(-1, -1);
	
	UE_LOG(LogTemp, Log, TEXT("BuildManager: Delete Mode set to %s"), bIsActive ?
		TEXT("Active") : TEXT("InActive"));
}

void ABuildManager::OnPlayerRotating(bool bIsRotating)
{
	bPlayerRotating = bIsRotating;

	if (TowerPreviewInstance)
	{
		SetActorHiddenInGame(bIsRotating);
	}
}

void ABuildManager::UpdatePreview()
{
	if(!GridManager)
	{ 
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: Missing GridManager!"));
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		FVector2D GridCoords;
		if (GridManager->WorldToGrid(HitResult.Location, GridCoords))
		{
			// Reset prev hover if changed
			if (GridCoords != LastHoveredTile && LastHoveredTile.X >= 0 && LastHoveredTile.Y >= 0)
			{
				FTileData& PrevTile = GridManager->TileGrid[LastHoveredTile.X][LastHoveredTile.Y];


				// Reset to base visual (based on mode and occupancy)
				ETileVisualState BaseState = PrevTile.VisualState; // Already set by mode toggle

				if (PrevTile.VisualState == ETileVisualState::Highlighted)
				{
					if (bBuildModeActive)
					{
						BaseState = (PrevTile.Occupancy == ETileOccupancyState::Empty) ? 
							ETileVisualState::Buildable : ETileVisualState::Occupied;
					}
					else if (bDeleteModeActive)
					{
						BaseState = (PrevTile.Occupancy == ETileOccupancyState::Tower) ?
							ETileVisualState::Occupied : ETileVisualState::Blocked;
					}
					GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, BaseState);
				}
			}

			LastHoveredTile = GridCoords;

			const FTileData& Tile = GridManager->TileGrid[GridCoords.X][GridCoords.Y];
			
			// Hover highlight if valid for mode
			bool bCanHighlight = false;
			if (bBuildModeActive && Tile.Occupancy == ETileOccupancyState::Empty)
			{
				bCanHighlight = true;
			}
			else if (bDeleteModeActive && Tile.Occupancy == ETileOccupancyState::Tower)
			{
				bCanHighlight = true;
			}

			if (bCanHighlight)
			{
				GridManager->SetTileVisual(GridCoords.X, GridCoords.Y, ETileVisualState::Highlighted); // Hover feedback (i should not have implemented this, took to much time, well well)
			}

			// Update prev actor (build mode VIP)
			if (bBuildModeActive && TowerPreviewInstance)
			{
				FVector NewLocation = Tile.WorldLocation + FVector(0.f, 0.f, 5.f); // Slightly above ground
				TowerPreviewInstance->SetActorLocation(NewLocation);
				TowerPreviewInstance->SetActorHiddenInGame(false);
			}
			else if (TowerPreviewInstance)
			{
				TowerPreviewInstance->SetActorHiddenInGame(true);
			}

			return;
		}
	}

	// Outside Grid: reset hover highlight
	if (LastHoveredTile.X >= 0 && LastHoveredTile.Y >= 0)
	{
		FTileData& PrevTile = GridManager->TileGrid[LastHoveredTile.X][LastHoveredTile.Y];
		ETileVisualState BaseState = PrevTile.VisualState;

		if (PrevTile.VisualState == ETileVisualState::Highlighted)
		{
			if (bBuildModeActive)
			{
				BaseState = (PrevTile.Occupancy == ETileOccupancyState::Empty) ?
					ETileVisualState::Buildable : ETileVisualState::Occupied;
			}
			else if (bDeleteModeActive)
			{
				BaseState = (PrevTile.Occupancy == ETileOccupancyState::Tower) ?
					ETileVisualState::Occupied : ETileVisualState::Blocked;
			}
			GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, BaseState);
		}
	}

	LastHoveredTile = FVector2D(-1, -1);

	if (TowerPreviewInstance)
	{
		TowerPreviewInstance->SetActorHiddenInGame(true);
	}
}

bool ABuildManager::TryPlaceTower()
{
	if (!GridManager || LastHoveredTile.X < 0 || LastHoveredTile.Y < 0)
		return false;

	FTileData& Tile = GridManager->TileGrid[LastHoveredTile.X][LastHoveredTile.Y];


	// Check if tile is buildable
	if (Tile.Occupancy != ETileOccupancyState::Empty)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: Tile at X=%d, Y=%d is not empty!"), (int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
		return false;
	}
	
	// Check for selected tower class
	if (!SelectedTowerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: No selected tower class!"));
		return false;
	}

	// --- Spawn Tower ---
	FVector SpawnLocation = Tile.WorldLocation + FVector(0, 0, 5.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;

	AActor* SpawnedTower = GetWorld()->SpawnActor<AActor>(SelectedTowerClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (!SpawnedTower)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildManager: Failed to spawn tower!"));
		return false;
	}


	// --- Save info ---
	FTowerData NewTower;
	NewTower.GridLocation = LastHoveredTile;
	NewTower.TowerActor = SpawnedTower;
	PlacedTowers.Add(NewTower);


	// --- Update Tile Logic ---
	Tile.Occupancy = ETileOccupancyState::Tower;
	GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, ETileVisualState::Occupied);

	UE_LOG(LogTemp, Log, TEXT("BuildManager: Tower placed at X=%d, Y=%d"), (int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
	return true;
}

bool ABuildManager::TryDeleteTower()
{
	if (!GridManager || LastHoveredTile.X < 0 || LastHoveredTile.Y < 0)
		return false;

	FTileData& Tile = GridManager->TileGrid[LastHoveredTile.X][LastHoveredTile.Y];

	// Check if tile has a tower
	if (Tile.Occupancy != ETileOccupancyState::Tower)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: No tower to delete at tile X=%d, Y=%d"), (int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
		return false;
	}

	// Find the tower in PlacedTowers
	for (int32 i = 0; i < PlacedTowers.Num(); ++i)
	{
		if (PlacedTowers[i].GridLocation == LastHoveredTile)
		{
			// Destroy the tower actor
			AActor* Tower = PlacedTowers[i].TowerActor;
			if (Tower)
			{
				Tower->Destroy();
			}

			PlacedTowers.RemoveAt(i);

			// Update Tile State
			Tile.Occupancy = ETileOccupancyState::Empty;

			// Set Visual based on CURRENT mode (doesnt really matter)
			ETileVisualState NewVisualState = ETileVisualState::Default;
			if (bBuildModeActive)
			{
				NewVisualState = ETileVisualState::Buildable;
			}
			else if (bDeleteModeActive)
			{
				NewVisualState = ETileVisualState::Blocked;
			}
			GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, NewVisualState);

			UE_LOG(LogTemp, Log, TEXT("BuildManager: Tower deleted at X=%d, Y=%d"), (int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("BuildManager: Tower data not found for deletion at X=%d, Y=%d!"), (int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
	return false;
}

void ABuildManager::SetGridManager(AGridManager* InGridManager)
{
	GridManager = InGridManager;
}

void ABuildManager::BeginPlay()
{
	Super::BeginPlay();
}

void ABuildManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GridManager && (bBuildModeActive || bDeleteModeActive) && !bPlayerRotating)
	{
		UpdatePreview();
	}
}