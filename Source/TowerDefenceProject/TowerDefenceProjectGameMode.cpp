// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenceProjectGameMode.h"
#include "Player/TopDownPawn.h"
#include "Player/PlayerResourceState.h"

ATowerDefenceProjectGameMode::ATowerDefenceProjectGameMode()
{
	DefaultPawnClass = ATopDownPawn::StaticClass();
	PlayerStateClass = APlayerResourceState::StaticClass();
}
