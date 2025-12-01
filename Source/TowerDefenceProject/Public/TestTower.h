// Fill out your copyright notice in the Description page of Project Settings.

//Talk to Even about getting the enemy movement so you can debug the overlap events!

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TestTower.generated.h"

class AProjectilePool;
class UTowerNode;
class ATowerTreeManager;
class AEnemyBase;
class USphereComponent;
class USceneComponent;

UCLASS()
class TOWERDEFENCEPROJECT_API ATestTower : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestTower();
	
	// Halvor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Stats|Economy")
	int32 TowerCost = 100;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tower")
	int32 GetTowerCost() const { return TowerCost; }
	
	// Preview hook: BP override for translucent/outline
	UFUNCTION(BlueprintCallable, Category = "Tower")
	virtual void SetPreviewMode(bool bIsPreview);
	
	void Test(); //REMOVE
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	//USceneComponent* TowerRoot;

	FTimerHandle FireRateHandle;
	void ResetTimer();

	//BPSphere and BPTowerMesh are set during construction in the BluePrint class for the tower. Sphere is for spawning bullets in the right place on the mesh, and TowerMesh is for clickable events.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomMesh")
	UStaticMeshComponent* BPSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomMesh")
	UStaticMeshComponent* BPTowerMesh;

	UPROPERTY()
	TSoftObjectPtr<UTowerNode> RootNodeRef = TSoftObjectPtr<UTowerNode>(FSoftObjectPath("/Game/Philip/Towers/TestTower/TestTower_UpgradeTree/TestTowerRoot.TestTowerRoot"));

	UPROPERTY(VisibleAnywhere, Category = "Tower")
	UTowerNode* CurrentNode = nullptr;

	AProjectilePool* ProjectilePool = nullptr;
	ATowerTreeManager* TowerTreeManager = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Tower")
	float FireRate = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Tower")
	float Range = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Tower")
	float Strength = 1.0f;
	
	UPROPERTY(VisibleAnywhere, Category = "Tower")
	AEnemyBase* CurrentTarget = nullptr;
	
	UPROPERTY(VisibleAnywhere, Category = "Tower")
	USphereComponent* EnemyDetector = nullptr;
	
	bool bIsPlaced = true;
	
	TArray<AEnemyBase*> EnemyArray;

	UFUNCTION()
	void GetUpgradeUI(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);
	
	UFUNCTION()
	void OnEnemyFound(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEnemyLost(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex);
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Fire();
};
