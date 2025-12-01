#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/GridManager.h"
#include "Player/PlayerResourceState.h"
#include "TowerData.h"
#include "Tower.h"
#include "BuildManager.generated.h"

UENUM(BlueprintType)
enum class EGameMode : uint8
{
	None,
	Build,
	Delete
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModeChangedDelegate, EGameMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTowerSelectedDelegate, int32, TowerIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTowerPlacedDelegate, FVector2D, Location, int32, Cost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTowerDeleteDelegate, FVector2D, Location, int32, Refund);

UCLASS()
class TOWERDEFENCEPROJECT_API ABuildManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABuildManager();
	
	// References
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	AGridManager* GridManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Towers", meta = (ClampMax = 9)); // clamp max towers
	TArray<TSubclassOf<ATower>> AvailableTowers;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	int32 SelectedTowerIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	int32 SelectedTowerCost = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Mode")
	EGameMode CurrentMode = EGameMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Economy")
	float RefundPercentage = 0.7f;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnModeChangedDelegate OnModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTowerSelectedDelegate OnTowerSelected;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTowerPlacedDelegate OnTowerPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTowerDeleteDelegate OnTowerDeleted;

	// Public API
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SelectTowerIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Building")
	int32 GetSelectedTowerCost() const { return SelectedTowerCost; }

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetMode(EGameMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool TryPlaceTower();

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool TryDeleteTower();

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsBuildModeActive() const { return CurrentMode == EGameMode::Build; }

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsDeleteModeActive() const { return CurrentMode == EGameMode::Delete; }

	UFUNCTION(BlueprintCallable, Category = "Building")
	ETileVisualState GetVisualStateForTile(const FTileData& Tile, bool bIsHovered) const;

	UFUNCTION(Exec, Category = "Debug") // PIE: "DebugPlaceTower"
	void DebugPlaceTower();

	UFUNCTION(Exec, Category = "Debug") // PIE: "DebugDeleteTower"
	void DebugDeleteTower();

	// Setters
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetGridManager(AGridManager* InGridManager);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetPlayerResourceState(APlayerResourceState* InPlayerResource);

	UFUNCTION()
	void OnPlayerRotating(bool bIsRotating);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// Constants
	static constexpr float TowerZOffset = 5.0f;

	// State
	FVector2D LastHoveredTile = FVector2D(-1, -1);
	bool bPlayerRotating = false;
	TSubclassOf<ATower> SelectedTowerClass;
	ATower* PreviewTower = nullptr;
	APlayerResourceState* PlayerResource = nullptr;
	TArray<FTowerData> PlacedTowers;

	// Helpers: Preview
	UFUNCTION()
	void DestroyAndNullPreview();

	UFUNCTION()
	void SpawnAndSetupPreview();

	UFUNCTION()
	void UpdatePreviewPosition(const FTileData& Tile);

	// Helpers: Hover/Visuals
	UFUNCTION()
	bool ValidateHoveredTile(FTileData& OutTile);

	UFUNCTION()
	void ResetPreviousHover();

	UFUNCTION()
	void UpdateHoverVisual(const FVector2D& GridCoords, bool bIsHovered);

	// Helpers: Placement/Delete
	UFUNCTION()
	int32 FindTowerIndex(const FVector2D& GridLocation) const;

	// For BP | HUD | UI
	UFUNCTION(BlueprintPure, Category = "Building")
	int32 GetTowerIndexAt(const FVector2D GridLocation) const; // -1 = not found

	UFUNCTION()
	int32 CalculateRefund(int32 OriginalCost) const;

	UFUNCTION()
	bool PerformPlacement(const FTileData& Tile);

	UFUNCTION()
	bool PerformDeletion(const FTileData& Tile);

	// Helper if need struct without copy within c++
	const FTowerData* GetTowerDataByIndex(int32 Index) const
	{
		return (Index != -1 && PlacedTowers.IsValidIndex(Index))
			? &PlacedTowers[Index]
			: nullptr;
	}

	// Update loop
	UFUNCTION()
	void UpdatePreview();
};
