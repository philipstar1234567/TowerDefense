
#include "Grid/BuildManager.h"

// Constructor
ABuildManager::ABuildManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABuildManager::SetBuildModeActive(bool bIsActive)
{
	BuildModeActive = bIsActive;

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

	//UE_LOG(LogTemp, Log, TEXT("Updating Preview"));

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		//UE_LOG(LogTemp, Log, TEXT("Cursor hit somehting at location: %s"), *HitResult.Location.ToString());

		FVector2D GridCoords;
		if (GridManager->WorldToGrid(HitResult.Location, GridCoords))
		{
			//UE_LOG(LogTemp, Log, TEXT("WorldToGrid succeeded. GridCoords: X=%d, Y=%d"), (int32)GridCoords.X, (int32)GridCoords.Y);
			LastHoveredTile = GridCoords;

			// Update Position
			const FTileData& Tile = GridManager->TileGrid[GridCoords.X][GridCoords.Y];
			FVector NewLocation = Tile.WorldLocation + FVector(0.f, 0.f, 5.f); // Slightly above ground
			
			if (TowerPreviewInstance)
			{
				TowerPreviewInstance->SetActorLocation(NewLocation);
				TowerPreviewInstance->SetActorHiddenInGame(false);
			}

			return;
		}
	}

	// Outside Grid
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

	if (Tile.bIsOccupied)
		return false;
	
	if (!SelectedTowerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildManager: No selected tower class!"));
		return false;
	}

	// Spawn Tower
	FVector SpawnLocation = Tile.WorldLocation + FVector(0, 0, 5.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	AActor* SpawnedTower = GetWorld()->SpawnActor<AActor>(SelectedTowerClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (!SpawnedTower)
		return false;

	// Save info
	FTowerData NewTower;
	NewTower.GridLocation = LastHoveredTile;
	NewTower.TowerActor = SpawnedTower;
	PlacedTowers.Add(NewTower);

	// Mark tile as occupied && update color
	Tile.bIsOccupied = true;
	GridManager->SetTileState(LastHoveredTile.X, LastHoveredTile.Y, ETileState::Occupied);

	return true;
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

	if (GridManager && BuildModeActive && !bPlayerRotating)
	{
		UpdatePreview();
	}
}