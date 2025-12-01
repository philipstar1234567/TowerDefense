
#include "Grid/BuildManager.h"
#include "Kismet/GameplayStatics.h"

ABuildManager::ABuildManager()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentMode = EGameMode::None;
	SelectedTowerIndex = -1;
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

void ABuildManager::SetMode(EGameMode NewMode)
{
	if (NewMode == CurrentMode) return;

	DestroyAndNullPreview();
	ResetPreviousHover();
	CurrentMode = NewMode;

	if (CurrentMode == EGameMode::Build && AvailableTowers.Num() > 0)
	{
		SelectTowerIndex(0);
	}

	UE_LOG(LogTemp, Log, TEXT("BuildManager: Mode changed to %d"), (uint8)CurrentMode);
}

void ABuildManager::SelectTowerIndex(int32 NewIndex)
{
	if (NewIndex < 0 || NewIndex >= AvailableTowers.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid index %d (0-%d)"), NewIndex, AvailableTowers.Num() - 1);
		return;
	}

	SelectedTowerIndex = NewIndex;
	SelectedTowerClass = AvailableTowers[NewIndex];

	if (SelectedTowerClass)
	{
		ATestTower* DefaultObj = SelectedTowerClass->GetDefaultObject<ATestTower>();
		SelectedTowerCost = DefaultObj->GetTowerCost();
	}
	else
	{
		SelectedTowerCost = 0;
	}

	if (IsBuildModeActive())
	{
		DestroyAndNullPreview();
		SpawnAndSetupPreview();
	}

	OnTowerSelected.Broadcast(SelectedTowerIndex);
	UE_LOG(LogTemp, Log, TEXT("Selected tower %d, cost %d"), NewIndex, SelectedTowerCost)
}

bool ABuildManager::TryPlaceTower()
{
	FTileData Tile;
	if (!ValidateHoveredTile(Tile) || Tile.Occupancy != ETileOccupancyState::Empty)
	{
		return false;
	}
	return PerformPlacement(Tile);
}

bool ABuildManager::TryDeleteTower()
{
	FTileData Tile;
	if (!ValidateHoveredTile(Tile) || Tile.Occupancy != ETileOccupancyState::Tower)
	{
		return false;
	}
	return PerformDeletion(Tile);
}

void ABuildManager::DebugPlaceTower() { TryPlaceTower(); }
void ABuildManager::DebugDeleteTower() { TryDeleteTower(); }

// === PRIVATE HELPERS ===
void ABuildManager::DestroyAndNullPreview()
{
	if (PreviewTower)
	{
		PreviewTower->Destroy();
		PreviewTower = nullptr;
	}
}

void ABuildManager::SpawnAndSetupPreview()
{
	if (!SelectedTowerClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PreviewTower = GetWorld()->SpawnActor<ATestTower>(SelectedTowerClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
	
	if (PreviewTower)
	{
		PreviewTower->SetActorEnableCollision(false);
		PreviewTower->SetPreviewMode(true);
	}
}

void ABuildManager::UpdatePreviewPosition(const FTileData& Tile)
{
	if (PreviewTower)
	{
		FVector NewLoc = Tile.WorldLocation + FVector(0.f, 0.f, TowerZOffset);
		PreviewTower->SetActorLocation(NewLoc);
		PreviewTower->SetActorHiddenInGame(false);
	}
}

bool ABuildManager::ValidateHoveredTile(FTileData& OutTile)
{
	return GridManager && LastHoveredTile.X >= 0 && LastHoveredTile.Y >= 0 &&
		GridManager->GetTileSafe(LastHoveredTile.X, LastHoveredTile.Y, OutTile);
}

void ABuildManager::ResetPreviousHover()
{
	if (LastHoveredTile.X >= 0 && LastHoveredTile.Y >= 0)
	{
		FTileData PrevTile;
		if (GridManager->GetTileSafe(LastHoveredTile.X, LastHoveredTile.Y, PrevTile) &&
			PrevTile.InternalVisualState == static_cast<uint8>(ETileVisualState::Highlighted))
		{
			ETileVisualState BaseState = GetVisualStateForTile(PrevTile, false);
			GridManager->SetTileVisual(LastHoveredTile.X, LastHoveredTile.Y, BaseState);
		}
	}

	LastHoveredTile = FVector2D(-1, -1);
	if (PreviewTower) PreviewTower->SetActorHiddenInGame(true);
}

void ABuildManager::UpdateHoverVisual(const FVector2D& GridCoords, bool bIsHovered)
{
	FTileData Tile;

	if (GridManager->GetTileSafe(GridCoords.X, GridCoords.Y, Tile))
	{
		ETileVisualState NewState = GetVisualStateForTile(Tile, bIsHovered);
		GridManager->SetTileVisual(GridCoords.X, GridCoords.Y, NewState);
	}
}

int32 ABuildManager::CalculateRefund(int32 OriginalCost) const
{
	return FMath::RoundToInt(static_cast<float>(OriginalCost) * RefundPercentage);
}

bool ABuildManager::PerformPlacement(const FTileData& Tile)
{
	if (!PlayerResource || !PlayerResource->SpendGold(SelectedTowerCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough gold! Need %d"), SelectedTowerCost);
		return false;
	}
	
	FVector SpawnLoc = Tile.WorldLocation + FVector(0.f, 0.f, TowerZOffset);
	FActorSpawnParameters Params;
	ATestTower* SpawnedTower = GetWorld()->SpawnActor<ATestTower>(SelectedTowerClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (!SpawnedTower)
	{
		PlayerResource->AddGold(SelectedTowerCost);
		UE_LOG(LogTemp, Error, TEXT("Spawn failed! Refunded"))
		return false;
	}

	SpawnedTower->SetPreviewMode(false);
	SpawnedTower->SetActorEnableCollision(true);
	SpawnedTower->bIsPlaced = true;

	FTowerData NewTower{ LastHoveredTile, SpawnedTower, SelectedTowerCost };
	PlacedTowers.Add(NewTower);
	GridManager->SetTileOccupant(LastHoveredTile.X, LastHoveredTile.Y, SpawnedTower);

	OnTowerPlaced.Broadcast(LastHoveredTile, SelectedTowerCost);
	UE_LOG(LogTemp, Log, TEXT("Placed tower at (%d,%d)"),
		(int32)LastHoveredTile.X, (int32)LastHoveredTile.Y);
	return true;
}

bool ABuildManager::PerformDeletion(const FTileData& Tile)
{
	const int32 TowerIdx = FindTowerIndex(LastHoveredTile);
	if (TowerIdx < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Tower missing from PlacedTowers!"));
		return false;
	}

	// Safe access via index
	const FTowerData& TowerData = PlacedTowers[TowerIdx];

	// Destroy actor
	TWeakObjectPtr<AActor> TowerWeak = TowerData.TowerActor;
	if (!TowerWeak.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Stale tower - cleaned."));
		PlacedTowers.RemoveAt(TowerIdx);
		GridManager->SetTileOccupant(LastHoveredTile.X, LastHoveredTile.Y, nullptr);
		return true;
	}

	TowerWeak->Destroy();

	// Refund logic
	int32 Refund = CalculateRefund(TowerData.Cost);
	if (PlayerResource) PlayerResource->AddGold(Refund);

	// clean array + tile
	PlacedTowers.RemoveAt(TowerIdx);
	GridManager->SetTileOccupant(LastHoveredTile.X, LastHoveredTile.Y, nullptr);
	UpdateHoverVisual(LastHoveredTile, false); // Reset visuals

	OnTowerDeleted.Broadcast(LastHoveredTile, Refund);
	UE_LOG(LogTemp, Log, TEXT("Deleted tower at (%d, %d), refunded %d"),
		(int32)LastHoveredTile.X, (int32)LastHoveredTile.Y, Refund);
	return true;
}

void ABuildManager::UpdatePreview()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	FHitResult Hit;
	if (!PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		ResetPreviousHover();
		return;
	}

	FVector2D GridCoords;
	if (!GridManager->WorldToGrid(Hit.Location, GridCoords))
	{
		ResetPreviousHover();
		return;
	}

	// New hover
	if (GridCoords != LastHoveredTile)
	{
		ResetPreviousHover();
		LastHoveredTile = GridCoords;
	}
	
	FTileData Tile;
	if (!GridManager->GetTileSafe(GridCoords.X, GridCoords.Y, Tile)) return;

	// Update visual
	bool bCanInteract = (IsBuildModeActive() && Tile.Occupancy == ETileOccupancyState::Empty) ||
		(IsDeleteModeActive() && Tile.Occupancy == ETileOccupancyState::Tower);
	UpdateHoverVisual(GridCoords, bCanInteract);

	// Position preview
	if (IsBuildModeActive() && Tile.Occupancy == ETileOccupancyState::Empty)
	{
		UpdatePreviewPosition(Tile);
	}
	else if (PreviewTower)
	{
		PreviewTower->SetActorHiddenInGame(true);
	}
}

ETileVisualState ABuildManager::GetVisualStateForTile(const FTileData& Tile, bool bIsHovered) const
{
	/*=== HOVER FIRST ===*/
	if (bIsHovered)
	{
		if (IsBuildModeActive() && Tile.Occupancy == ETileOccupancyState::Empty)
			return ETileVisualState::Highlighted;
		if (IsDeleteModeActive() && Tile.Occupancy == ETileOccupancyState::Tower)
			return ETileVisualState::Highlighted;
	}

	/*=== NO MODE ACTIVE ===*/
	if (CurrentMode == EGameMode::None)
	{
		if (Tile.Occupancy == ETileOccupancyState::Spawn) return ETileVisualState::Spawn;
		if (Tile.Occupancy == ETileOccupancyState::Goal) return ETileVisualState::Goal;
		return ETileVisualState::Default;
	}

	/*=== BUILD MODE ===*/
	if (IsBuildModeActive())
	{	
		if (Tile.Occupancy == ETileOccupancyState::Empty) return ETileVisualState::Buildable;
		if (Tile.Occupancy == ETileOccupancyState::Tower) return ETileVisualState::Occupied;
		if (Tile.Occupancy == ETileOccupancyState::Spawn) return ETileVisualState::Spawn;
		if (Tile.Occupancy == ETileOccupancyState::Goal) return ETileVisualState::Goal;
		return ETileVisualState::Blocked;
	}

	/*=== DELETE MODE ===*/
	if (IsDeleteModeActive())
	{
		if (Tile.Occupancy == ETileOccupancyState::Tower) return ETileVisualState::Buildable;
		if (Tile.Occupancy == ETileOccupancyState::Spawn) return ETileVisualState::Spawn;
		if (Tile.Occupancy == ETileOccupancyState::Goal) return ETileVisualState::Goal;
		return ETileVisualState::Blocked;
	}

	return ETileVisualState::Default;
}

void ABuildManager::OnPlayerRotating(bool bIsRotating)
{
	bPlayerRotating = bIsRotating;
	if (PreviewTower) PreviewTower->SetActorHiddenInGame(bIsRotating);
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
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: Null PlayerResource!"));
	}

}

int32 ABuildManager::FindTowerIndex(const FVector2D& GridLocation) const
{
	for (int32 i = 0; i < PlacedTowers.Num(); i++)
	{
		if (PlacedTowers[i].GridLocation.Equals(GridLocation))
		{
			return i;
		}
	}

	return -1;
}

int32 ABuildManager::GetTowerIndexAt(const FVector2D GridLocation) const
{
	return FindTowerIndex(GridLocation);
}
