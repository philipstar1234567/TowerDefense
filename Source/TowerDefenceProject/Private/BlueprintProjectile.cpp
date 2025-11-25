// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintProjectile.h"

// Sets default values
ABlueprintProjectile::ABlueprintProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABlueprintProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABlueprintProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

