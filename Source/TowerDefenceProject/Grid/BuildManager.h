
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/GridManager.h"
#include "TowerData.h"
#include "BuildManager.generated.h"

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
	
	UFUNCTION(BlueprintCallable)
	void SetBuildModeActive(bool bIsActive);

	UFUNCTION()
	void OnPlayerRotating(bool bIsRotating);
	
	bool TryPlaceTower();

	void UpdatePreview();
	
	// Setter funciton
	void SetGridManager(AGridManager* InGridManager);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	FVector2D LastHoveredTile = FVector2D(-1, -1);
	bool BuildModeActive = false;
	bool bPlayerRotating = false;
};
