// Fill out your copyright notice in the Description page of Project Settings.


#include "TestPlayerController.h"
#include "TestTower.h"
#include "Kismet/GameplayStatics.h"

void ATestPlayerController::TestTower()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "TestTower");
	ATestTower* Tower = Cast<ATestTower>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestTower::StaticClass()));
	Tower->Test();
	bEnableClickEvents = true;
	bShowMouseCursor = true;
}
