// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TestPlayerController.generated.h"

/**
 * @brief WAS USED FOR DEBUGGING, DON'T BOTHER LOOKING HERE
 */
UCLASS()
class TOWERDEFENCEPROJECT_API ATestPlayerController : public APlayerController
{
	GENERATED_BODY()

	UFUNCTION(Exec)
	void TestTower();
};
