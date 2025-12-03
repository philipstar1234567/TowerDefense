
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerResourceState.generated.h"

/**
* @brief broadcasts if player died (hp <= 0)
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverDelegate);


/**
* @brief sends signal if any stats changes so HUD/UI will know and update
* @param Gold The amount of gold player has
* @param Health The amount of health player has
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourcesChangedDelegate, int32, Gold, int32, Health);

/**
* @brief A player state that gives player "resources", like gold health, and has easy managing
*/
UCLASS()
class TOWERDEFENCEPROJECT_API APlayerResourceState : public APlayerState
{
	GENERATED_BODY()

public:
	/** @brief Constructor ... */
	APlayerResourceState();

	/** @brief Instance of the defeat delegate */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameOverDelegate OnGameOver;

	/** @brief Instance of the resource delegate */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnResourcesChangedDelegate OnResourcesChanged;

	/** @brief Starting gold value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Properties")
	int32 StartingGold = 300;

	/** @brief Starting health value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Properties")
	int32 StartingHealth = 10;

	// --- Functions ---
	/**
	* @brief Adds gold based on parameter
	* @param AddAmount The amount of gold to add
	*/
	UFUNCTION(BlueprintCallable)
	void AddGold(int32 AddAmount);

	/**
	* @brief Spends / Removes gold based on parameter
	* @param SpendAmount The amount of gold to remove
	* @return True if enough gold to remove
	*/
	UFUNCTION(BlueprintCallable)
	bool SpendGold(int32 SpendAmount);

	/**
	* @brief Deals damage based on amount and checks if dead
	* @param DamageAmount The amount of damage...
	*/
	UFUNCTION(BlueprintCallable)
	void ApplyDamage(int32 DamageAmount);

	/**
	* @brief Checks if dead
	* @return True if health is <= 0
	*/
	UFUNCTION(BlueprintCallable)
	bool IsGameOver();

	/**
	* @brief Gets gold amount
	* @return Amount of gold
	*/
	UFUNCTION(BlueprintCallable)
	int32 GetGoldAmount();

	/**
	* @brief Gets health amount
	* @return Amount of health
	*/
	UFUNCTION(BlueprintCallable)
	int32 GetHealthAmount();

	/**
	* @brief Runs from enemy when they enter goal and deals damage
	* @param dmg The amount of damage
	*/
	UFUNCTION()
	void HandleEnemyFinished(int32 dmg);

private:
	/**
	* @brief The amount of Gold
	*/
	UPROPERTY()
	int32 Gold;

	/**
	* @brief The amount of Health
	*/
	UPROPERTY()
	int32 Health;
};
