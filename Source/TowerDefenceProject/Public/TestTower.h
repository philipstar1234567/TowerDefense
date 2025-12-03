// Fill out your copyright notice in the Description page of Project Settings.

//Talk to Even about getting the enemy movement so you can debug the overlap events!

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UpgradeTreeUI.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TestTower.generated.h"

class AProjectilePool;
class UTowerNode;
class ATowerTreeManager;
class AEnemyBase;
class UUpgradeTreeUI;
class USphereComponent;
class USceneComponent;
class UCollisionSphere;
class ATopDownPawn;

UCLASS()
class TOWERDEFENCEPROJECT_API ATestTower : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestTower();
	
	// Cost to build tower
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Stats|Economy")
	int32 TowerCost = 100;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tower")
	int32 GetTowerCost() const { return TowerCost; }
	
	// Preview hook: BP override for translucent/outline
	UFUNCTION(BlueprintCallable, Category = "Tower")
	virtual void SetPreviewMode(bool bIsPreview);
	
	FTimerHandle FireRateHandle;
	void ResetTimer();

	// BPSphere and BPTowerMesh are set during construction in the BluePrint class for the tower. Sphere is for spawning bullets in the right place on the mesh, and TowerMesh is for clickable events.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomMesh")
	UStaticMeshComponent* BPSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomMesh")
	UStaticMeshComponent* BPCube;
	
	//Mesh that displays the range of the tower when building or upgrading
	UPROPERTY(EditAnywhere, Category="CustomMesh")
	UStaticMeshComponent* RangeMesh;

	// Function to fix the range display-mesh based on tower's current range
	void FixRangeMeshSize();

	// Tower's root node, gets set in BeginPlay so it's loaded
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower")
	TSoftObjectPtr<UTowerNode> RootNodeRef = TSoftObjectPtr<UTowerNode>(FSoftObjectPath("/Game/Philip/Towers/TestTower/TestTower_UpgradeTree/TestTowerRoot.TestTowerRoot"));

	UPROPERTY(EditAnywhere, Category = "Tower")
	UTowerNode* CurrentNode = nullptr;

	// References to other classes, set in BeginPlay so everything is loaded
	AProjectilePool* ProjectilePool = nullptr;
	ATowerTreeManager* TowerTreeManager = nullptr;
	ATopDownPawn* TopDownPawn = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Tower")
	USphereComponent* EnemyDetector = nullptr;
	
	// Actual instance of Upgrade UI
	UPROPERTY(VisibleAnywhere, Category = "Tower")
	UUpgradeTreeUI* UpgradeTreeUIInstance;
	
	// Class that gets passed as argument ATestTower, line 69. The class the UI must "fit into"
	UPROPERTY(EditAnywhere, Category = "Tower")
	TSubclassOf<UUpgradeTreeUI> UpgradeTreeUIClass;

	// Rate of fire for projectiles
	UPROPERTY(EditAnywhere, Category = "Tower")
	float FireRate = 0.5f;

	// The tower's range, as a radius
	UPROPERTY(EditAnywhere, Category = "Tower")
	float Range = 400.0f;

	// Amount of damage that should be dealt to hit enemies
	UPROPERTY(EditAnywhere, Category = "Tower")
	float Strength = 25.0f;
	
	// Bool if the tower has been placed or is just in preview mode
	UPROPERTY(EditAnywhere, Category = "Tower Stats")
	bool bIsPlaced = false;
	
	// Array of all enemies inside the tower's range, potential targets
	TArray<AEnemyBase*> EnemyArray;

	/**
	 * @brief Draws the upgrade UI on screen for the tower
	 * 
	 * @param ClickedComp Doesn't fucking matter, Unreal demands it
	 * @param ButtonPressed Doesn't fucking matter, Unreal demands it
	 */
	UFUNCTION()
	void GetUpgradeUI(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);

	/**
	 * @brief Adds enemies in range to the enemy array
	 * 
	 * @param EventGenerator Component generating the event
	 * @param FoundActor Actor that was collided with
	 * @param FoundComp Specific component inside actor that was collided with
	 * @param FoundBodyIndex Doesn't fucking matter, Unreal demands it
	 * @param bFromSweep Doesn't fucking matter, Unreal demands it
	 * @param SweepResult Doesn't fucking matter, Unreal demands it
	 */
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void OnEnemyFound(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * @brief Removes enemies out of range from the enemy array
	 * 
	 * @param EventGenerator Component generating the event
	 * @param FoundActor Actor that was collided with
	 * @param FoundComp Specific component inside actor that was collided with
	 * @param FoundBodyIndex Doesn't fucking matter, Unreal demands it
	 */
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void OnEnemyLost(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Gets projectile from projectilePool and fires it
	 * 
	 * If the projectilePool is empty, the tower will spawn a new
	 * projectile. Enable() is a function in the projectile that
	 * readies the projectile on it's end
	 */
	void Fire();
};
