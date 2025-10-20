
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/GridManager.h"
#include "TowerData.h"
#include "BuildManager.generated.h"

UENUM(BlueprintType)
enum class EGameMode : uint8
{
	None,
	Build,
	Delete
};

UCLASS()
class TOWERDEFENCEPROJECT_API ABuildManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABuildManager();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	AGridManager* GridManager;

	UPROPERTY(EditDefaultsOnly, Category = "Building")
	TSubclassOf<AActor> DefaultTowerClass;

	UPROPERTY()
	TSubclassOf<AActor> SelectedTowerClass;

	UPROPERTY()
	AActor* TowerPreviewInstance;

	UPROPERTY()
	TArray<FTowerData> PlacedTowers;

	UPROPERTY(BlueprintReadOnly, Category = "Mode")
	EGameMode CurrentMode = EGameMode::None;
	
	UFUNCTION(BlueprintCallable)
	void SetMode(EGameMode NewMode);

	UFUNCTION(BlueprintCallable)
	bool IsBuildModeActive() const { return CurrentMode == EGameMode::Build; }

	UFUNCTION(BlueprintCallable)
	bool IsDeleteModeActive() const { return CurrentMode == EGameMode::Delete; }

	UFUNCTION(BlueprintCallable)
	ETileVisualState GetVisualStateForTile(const FTileData& Tile, bool bIsHovered) const;

	UFUNCTION()
	void OnPlayerRotating(bool bIsRotating);
	
	bool TryPlaceTower();
	bool TryDeleteTower();

	void UpdatePreview();

	// Setter funciton
	void SetGridManager(AGridManager* InGridManager);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	FVector2D LastHoveredTile = FVector2D(-1, -1);
	bool bPlayerRotating = false;
};
