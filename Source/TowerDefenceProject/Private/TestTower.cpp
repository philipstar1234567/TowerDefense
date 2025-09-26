// Fill out your copyright notice in the Description page of Project Settings.


#include "TestTower.h"
#include "Components/SphereComponent.h"

// Sets default values
ATestTower::ATestTower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->InitSphereRadius(10.0f);
	//projectile actually needs to be its own cpp file since it needs to have independant spawning/movement... dumbass

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

}

