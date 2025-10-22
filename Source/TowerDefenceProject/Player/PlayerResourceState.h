
#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerState.h"
#include "PlayerResourceState.generated.h"

// Delegate for defeat
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverDelegate);

UCLASS()
class TOWERDEFENCEPROJECT_API APlayerResourceState : public APlayerState
{
	GENERATED_BODY()

public:
	APlayerResourceState();

	// Instance of the delegate
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameOverDelegate OnGameOver;

	// Starting Values
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Properties")
	int32 StartingGold = 200;

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

private:
	// --- Variables ---
	UPROPERTY()
	int32 Gold;

	UPROPERTY()
	int32 Health;
};
