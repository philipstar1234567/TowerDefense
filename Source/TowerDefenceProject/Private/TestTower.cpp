// Fill out your copyright notice in the Description page of Project Settings.


#include "TestTower.h"
#include "StandardProjectile.h"
#include "Components/SphereComponent.h"

// Sets default values
ATestTower::ATestTower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATestTower::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATestTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FFireObject.Broadcast(this);
	//GetWorld()->SpawnActor<AStandardProjectile>(FVector(-350.0f, -130.0f, 300.0f), FRotator(90, 0, 0));

}

void ATestTower::Fire()
{
	GetWorld()->SpawnActor<AStandardProjectile>();
}