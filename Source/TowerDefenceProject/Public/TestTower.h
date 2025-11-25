// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TestTower.generated.h"

class AProjectilePool;
class UTowerNode;
class ATowerTreeManager;

UCLASS()
class TOWERDEFENCEPROJECT_API ATestTower : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestTower();
	
	void Test(); //REMOVE

	FTimerHandle FireRateHandle;
	void ResetTimer();

	//BPSphere and BPTowerMesh are set during construction in the BluePrint class for the tower. Sphere is for spawning bullets in the right place on the mesh, and TowerMesh is for clickable events.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomMesh")
	UStaticMeshComponent* BPSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomMesh")
	UStaticMeshComponent* BPTowerMesh;

	UPROPERTY()
	TSoftObjectPtr<UTowerNode> RootNodeRef = TSoftObjectPtr<UTowerNode>(FSoftObjectPath("/Game/Towers/TestTower/TestTower_UpgradeTree/TestTowerRoot.TestTowerRoot"));

	UPROPERTY(VisibleAnywhere, Category = "Tower")
	UTowerNode* CurrentNode = nullptr;

	AProjectilePool* ProjectilePool = nullptr;
	ATowerTreeManager* TowerTreeManager = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Tower")
	float FireRate = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Tower")
	float Range = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Tower")
	float Strength = 1.0f;

	UFUNCTION()
	void GetUpgradeUI(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Fire();
};
