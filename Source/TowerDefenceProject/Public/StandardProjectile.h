// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TestTower.h"
#include "EnemyBase.h"
#include "Components/SphereComponent.h"
#include "Math/Rotator.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "StandardProjectile.generated.h"

class AProjectilePool;
class AEnemyBase;

/**
 * @brief Projectile fired by towers
 * 
 * Created completely in C++, no blueprint child exists. Stored in
 * the scene's projectilePool.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API AStandardProjectile : public AActor
{
	GENERATED_BODY()
	
	// Component to check for collisions with ABaseEnemy
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComp;
	
	// Component responsible for moving the object
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 2000.0f;
	
	UPROPERTY(EditAnywhere)
	float Size = 100.0f;
	
	// Strength is set by the tower calling the projectile, determines damage
	float Strength;
	
public:	
	// Sets default values for this actor's properties
	AStandardProjectile();

	// Reference to projectilePool
	UPROPERTY()
	AProjectilePool* ProjectilePool;

	// Timer that deactivates the projectile and puts them back in the projectilePool
	FTimerHandle ProjectileLifespanHandle;

	UPROPERTY(EditAnywhere, Category="Custom")
	float ProjectileLifespan = 5.0f;

	/**
	 * @brief Handles collision with other collision components.
	 * 
	 * Checks if collided with AEnemyBase, if so, calls the damage function
	 * the enemy class has.
	 * 
	 * @param EventGenerator Collision component from projectile
	 * @param FoundActor The actor that was hit
	 * @param FoundComp The component in the actor that was hit
	 * @param FoundBodyIndex Doesn't fucking matter, Unreal demands it
	 * @param bFromSweep Doesn't fucking matter, Unreal demands it
	 * @param SweepResult Doesn't fucking matter, Unreal demands it
	 */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//Deactivates the projectile (hides and turns off collisions) after the lifespan is up, and puts it back in the projectilePool
	void Disable();

	/**
	 * @brief Activates a projectile (turns on visibilty and collisions) from a tower.
	 * 
	 * This function assumes the tower calling it has already popped it
	 * from the array! Had to be done that way, originally had problems with multiple towers calling
	 * a projectile before the projectile had time to remove itself from the array.
	 * 
	 * @param SpawnTower Tower that activates the projectile
	 * @param Direction The direction to fire in
	 */
	void Enable(ATestTower* SpawnTower, FVector Direction);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
