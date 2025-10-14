// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TestTower.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class TOWERDEFENCEPROJECT_API ATestTower : public AActor
{
	GENERATED_BODY()

	//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFireDelegate, ATestTower*, Tower);
	
public:	
	// Sets default values for this actor's properties
	ATestTower();

	UPROPERTY()
	FFireDelegate FFireObject;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Fire();

};
