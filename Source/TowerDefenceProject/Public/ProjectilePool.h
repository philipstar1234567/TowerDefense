// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectilePool.generated.h"

class AStandardProjectile;

/**
 * @brief A class that stores all spawned projectiles for later reuse.
 * 
 * The class has an array of AStandardProjectiles, that gets pushed and popped in ATestTower.
 * None of the logic exists in this class, it only exists to hold an array.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API AProjectilePool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectilePool();
	
	UPROPERTY()
	TArray<AStandardProjectile*> ProjectilePool;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
