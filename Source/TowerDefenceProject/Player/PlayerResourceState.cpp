

#include "Player/PlayerResourceState.h"

APlayerResourceState::APlayerResourceState()
{
	Gold = StartingGold;
	Health = StartingHealth;
	UE_LOG(LogTemp, Log, TEXT("PlayerResourceState: Initialized with Gold=%d, Health=%d"),
		Gold, Health);
}

void APlayerResourceState::AddGold(int32 AddAmount)
{
	if (0 > AddAmount)
		return;
	
	Gold += AddAmount;

	UE_LOG(LogTemp, Log, TEXT("PlayerResourceState: Added %d amount of Gold"), AddAmount);
}

bool APlayerResourceState::SpendGold(int32 SpendAmount)
{
	if (SpendAmount < 0)
		return false;

	if (SpendAmount > Gold)
		return false;

	Gold -= SpendAmount;
	return true;
}

void APlayerResourceState::ApplyDamage(int32 DamageAmount)
{
	if (DamageAmount < 0)
		return;

	Health -= DamageAmount;

	if (Health < 0)
	{
		Health = 0;
	}
	
	// Check if game is over
	if (IsGameOver())
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerResourceState: Game Over!"));

		// Activate a delegate
		OnGameOver.Broadcast();
	}
}

bool APlayerResourceState::IsGameOver()
{
	return Health <= 0;
}

int32 APlayerResourceState::GetGoldAmount()
{
	return Gold;
}

int32 APlayerResourceState::GetHealthAmount()
{
	return Health;
}
