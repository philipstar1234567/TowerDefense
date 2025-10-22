
#include "Grid/BuildManager.h"

// Constructor
ABuildManager::ABuildManager()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentMode = EGameMode::None;
}

void ABuildManager::SetMode(EGameMode NewMode)
{
	if (NewMode == CurrentMode) return;

	// Clean prev mode
	if (TowerPreviewInstance)
	{
		TowerPreviewInstance->Destroy();
		TowerPreviewInstance = nullptr;
		UE_LOG(LogTemp, Log, TEXT("BuildManager: Tower preview destroyed."));
	}

	LastHoveredTile = FVector2D(-1, -1);

	CurrentMode = NewMode;

	if (CurrentMode == EGameMode::Build)
	{
		// Choose the selected tower class (fallback to default)
		if (!SelectedTowerClass)
			SelectedTowerClass = DefaultTowerClass;

		if (SelectedTowerClass)
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

	UE_LOG(LogTemp, Log, TEXT("BuildManager: Tower preview spawned."));
}

ETileVisualState ABuildManager::GetVisualStateForTile(const FTileData& Tile, bool bIsHovered) const
{
	if (bIsHovered && ((CurrentMode == EGameMode::Build && Tile.Occupancy == ETileOccupancyState::Empty) ||
		(CurrentMode == EGameMode::Delete && Tile.Occupancy == ETileOccupancyState::Tower)))
	{
		return ETileVisualState::Highlighted;
	}

	if (CurrentMode == EGameMode::Build)
	{
		if (Tile.Occupancy == ETileOccupancyState::Empty) return ETileVisualState::Buildable;
		if (Tile.Occupancy == ETileOccupancyState::Tower) return ETileVisualState::Occupied;
		if (Tile.Occupancy == ETileOccupancyState::Path) return ETileVisualState::Blocked;
	}
	else if (CurrentMode == EGameMode::Delete)
	{
		if (Tile.Occupancy == ETileOccupancyState::Tower) return ETileVisualState::Occupied;
		return ETileVisualState::Blocked;
	}

	return ETileVisualState::Default;
}

void ABuildManager::OnPlayerRotating(bool bIsRotating)
{
	bPlayerRotating = bIsRotating;
	if (TowerPreviewInstance)
		TowerPreviewInstance->SetActorHiddenInGame(bIsRotating);
}

void ABuildManager::UpdatePreview()
{
	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: Missing GridManager!"));
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: UpdatePreview failed - PlayerController is null"))
		return;
	}

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		FVector2D GridCoords;
		if (GridManager->WorldToGrid(HitResult.Location, GridCoords))
		{
			// Reset prev hover if changed
			if (GridCoords != LastHoveredTile && LastHoveredTile.X >= 0 && LastHoveredTile.Y >= 0)
			{
				FTileData PrevTile;
				if (GridManager->GetTileSafe(LastHoveredTile.X, LastHoveredTile.Y, PrevTile))
				{
					ETileVisualState BaseState = GetVisualStateForTile(PrevTile, false);
					if (PrevTile.VisualState == ETileVisualState::Highlighted)
					{
						GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, BaseState);
					}
				}
			}

			LastHoveredTile = GridCoords;
			FTileData Tile;
			if (GridManager->GetTileSafe(GridCoords.X, GridCoords.Y, Tile))
			{
				bool bCanHighlight = (CurrentMode == EGameMode::Build && Tile.Occupancy == ETileOccupancyState::Empty) ||
					(CurrentMode == EGameMode::Delete && Tile.Occupancy == ETileOccupancyState::Tower);

				if (bCanHighlight)
				{
					GridManager->SetTileVisual(GridCoords.X, GridCoords.Y, GetVisualStateForTile(Tile, true));
				}

				if (CurrentMode == EGameMode::Build && TowerPreviewInstance && Tile.Occupancy == ETileOccupancyState::Empty)
				{
					FVector NewLocation = Tile.WorldLocation + FVector(0.f, 0.f, 5.f);
					TowerPreviewInstance->SetActorLocation(NewLocation);
					TowerPreviewInstance->SetActorHiddenInGame(false);
				}
				else if (TowerPreviewInstance)
				{
					TowerPreviewInstance->SetActorHiddenInGame(true);
				}
			}
			return;
		}
	}

	// Outside Grid: reset hover highlight
	if (LastHoveredTile.X >= 0 && LastHoveredTile.Y >= 0)
	{
		FTileData PrevTile;
		if (GridManager->GetTileSafe(LastHoveredTile.X, LastHoveredTile.Y, PrevTile))
		{
			ETileVisualState BaseState = GetVisualStateForTile(PrevTile, false);
			if (PrevTile.VisualState == ETileVisualState::Highlighted)
			{
				GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, BaseState);
			}
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

	FTileData Tile;
	if (!GridManager->GetTileSafe(LastHoveredTile.X, LastHoveredTile.Y, Tile))
		return false;

	if (Tile.Occupancy != ETileOccupancyState::Empty)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: Tile at X=%d, Y=%d is not empty!"),
			(int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
		return false;
	}

	if (!SelectedTowerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: No selected tower class!"));
		return false;
	}

	// --- Check and Spend gold ---
	if (!PlayerResource || !PlayerResource->SpendGold(TowerCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: Not enough gold to place tower! Current gold: %d, Cost: %d"),
			PlayerResource ? PlayerResource->GetGoldAmount() : 0, TowerCost);
		// Add UI notify here if needed in future
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

		// refund gold
		if (PlayerResource)
			PlayerResource->AddGold(TowerCost);

		return false;
	}

	// --- Save info ---
	FTowerData NewTower;
	NewTower.GridLocation = LastHoveredTile;
	NewTower.TowerActor = SpawnedTower;
	NewTower.Cost = TowerCost;
	PlacedTowers.Add(NewTower);

	// --- Update Tile in Grid ---
	GridManager->SetTileOccupancy(LastHoveredTile.X, LastHoveredTile.Y, ETileOccupancyState::Tower);

	// Re-fetch updated tile for visual calc
	FTileData UpdatedTile;
	if (GridManager->GetTileSafe(LastHoveredTile.X, LastHoveredTile.Y, UpdatedTile))
	{
		GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, GetVisualStateForTile(UpdatedTile, false));
	}

	UE_LOG(LogTemp, Log, TEXT("BuildManager: Tower placed at X=%d, Y=%d"),
		(int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
	return true;
}

bool ABuildManager::TryDeleteTower()
{
	if (!GridManager || LastHoveredTile.X < 0 || LastHoveredTile.Y < 0)
		return false;

	FTileData Tile;
	if (!GridManager->GetTileSafe(LastHoveredTile.X, LastHoveredTile.Y, Tile))
		return false;

	if (Tile.Occupancy != ETileOccupancyState::Tower)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: No tower to delete at tile X=%d, Y=%d"),
			(int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
		return false;
	}

	for (int32 i = 0; i < PlacedTowers.Num(); ++i)
	{
		if (PlacedTowers[i].GridLocation == LastHoveredTile)
		{
			AActor* Tower = PlacedTowers[i].TowerActor;
			if (Tower)
			{
				Tower->Destroy();
			}

			// Calc & add refund
			int32 Refund = FMath::RoundToInt(static_cast<float>(PlacedTowers[i].Cost) * RefundPercentage); // i think this works, might be the error u are looking for
			if (PlayerResource)
			{
				PlayerResource->AddGold(Refund);
				UE_LOG(LogTemp, Log, TEXT("BuildManager: Refunded %d gold (%.0f%% of %d)"),
					Refund, RefundPercentage * 100.f, PlacedTowers[i].Cost);
			}

			PlacedTowers.RemoveAt(i);

			// --- Update Tile in Grid ---
			GridManager->SetTileOccupancy(LastHoveredTile.X, LastHoveredTile.Y, ETileOccupancyState::Empty);

			// Re-fetch updated tile for visual calc
			FTileData UpdatedTile;
			if (GridManager->GetTileSafe(LastHoveredTile.X, LastHoveredTile.Y, UpdatedTile))
			{
				GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, GetVisualStateForTile(UpdatedTile, false));
			}
			
			UE_LOG(LogTemp, Log, TEXT("BuildManager: Tower deleted at X=%d, Y=%d"),
				(int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("BuildManager: Tower data not found for deletion at X=%d, Y=%d!"),
		(int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
	return false;
}

void ABuildManager::SetGridManager(AGridManager* InGridManager)
{
	GridManager = InGridManager;
}

void ABuildManager::SetPlayerResourceState(APlayerResourceState* InPlayerResource)
{
	PlayerResource = InPlayerResource;
	if (!PlayerResource)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: SetPlayerResourceState received null!"));
	}

}

void ABuildManager::BeginPlay()
{
	Super::BeginPlay();
}

void ABuildManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GridManager && CurrentMode != EGameMode::None && !bPlayerRotating)
	{
		UpdatePreview();
	}
}