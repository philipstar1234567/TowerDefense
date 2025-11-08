// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TestTower.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "StandardProjectile.generated.h"

class AProjectilePool;

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

	UPROPERTY(VisibleAnywhere)
	ATestTower* Tower;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 500.0f;

	UPROPERTY(EditAnywhere)
	FVector Direction = FVector(1.f, 0.f, 0.f);
	
public:	
	// Sets default values for this actor's properties
	AStandardProjectile();

	UPROPERTY()
	AProjectilePool* ProjectilePool;

	FTimerHandle ProjectileLifespanHandle;

	UPROPERTY(EditAnywhere)
	float ProjectileLifespan = 2.0f;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void Spawn(ATestTower* SpawnTower);

	void Disable();
	void Enable(ATestTower* SpawnTower);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
