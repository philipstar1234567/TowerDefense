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

UCLASS()
class TOWERDEFENCEPROJECT_API AStandardProjectile : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 2000.0f;
	
	UPROPERTY(EditAnywhere)
	float Size = 100.0f;
	
	float Strength;

	UPROPERTY(EditAnywhere)
	FRotator MovementDirection = FRotator(1.f, 0.f, 0.f);
	
public:	
	// Sets default values for this actor's properties
	AStandardProjectile();

	UPROPERTY()
	AProjectilePool* ProjectilePool;

	FTimerHandle ProjectileLifespanHandle;

	UPROPERTY(EditAnywhere, Category="Custom")
	float ProjectileLifespan = 5.0f;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Spawn(ATestTower* SpawnTower);

	void Disable();
	void Enable(ATestTower* SpawnTower, FVector Direction);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
