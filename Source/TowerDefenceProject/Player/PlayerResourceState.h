
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerResourceState.generated.h"

// Delegate for defeat
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverDelegate);

// Delegate for resource change - HUD will use it
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourcesChangedDelegate, int32, Gold, int32, Health);

UCLASS()
class TOWERDEFENCEPROJECT_API APlayerResourceState : public APlayerState
{
	GENERATED_BODY()

public:
	APlayerResourceState();

	// Instance of the defeat delegate
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameOverDelegate OnGameOver;

	// instance of the resource delegate
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnResourcesChangedDelegate OnResourcesChanged;

	// Starting Values
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Properties")
	int32 StartingGold = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Properties")
	int32 StartingHealth = 10;

	// --- Functions ---
	UFUNCTION(BlueprintCallable)
	void AddGold(int32 AddAmount);

	UFUNCTION(BlueprintCallable)
	bool SpendGold(int32 SpendAmount);

	UFUNCTION(BlueprintCallable)
	void ApplyDamage(int32 DamageAmount);

	UFUNCTION(BlueprintCallable)
	bool IsGameOver();

	// --- Get Functions ---
	UFUNCTION(BlueprintCallable)
	int32 GetGoldAmount();

	UFUNCTION(BlueprintCallable)
	int32 GetHealthAmount();

	UFUNCTION()
	void HandleEnemyFinished(int32 dmg);

private:
	// --- Variables ---
	UPROPERTY()
	int32 Gold;

	UPROPERTY()
	int32 Health;
};
