#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBase.generated.h"

class AEnemyHandler;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyFinishedDelegate, int32, DamageAmount);

UCLASS()
class TOWERDEFENCEPROJECT_API AEnemyBase : public AActor
{
    GENERATED_BODY()

public:
    AEnemyBase();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Called right after spawn by the WaveManager */
    void InitializeEnemy(AEnemyHandler* InEnemyHandler, const FVector& InTargetLocation);

    /** Called by handler to re-path (e.g., after tower placed) */
    UFUNCTION()
    void RecalculatePath();
    
    /** Component to check for collisions with TestTower's EnemyDetector and StandardProjectile */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USphereComponent* CollisionComp;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEnemyFinishedDelegate OnEnemyFinished;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoalDamage = 1;

protected:
    /** The path returned by EnemyHandler (list of world points) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    TArray<FVector> CurrentPath;

    /** Path target index we're currently moving toward */
    int32 CurrentPathIndex;

    /** Movement speed (units per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed;

    /** Distance threshold to "reach" a waypoint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WaypointAcceptanceRadius;

    /** Cached target world location */
    FVector TargetLocation;

    /** Reference to the global EnemyHandler (for pathfinding) */
    AEnemyHandler* EnemyHandler;

    /** Requests a new path from EnemyHandler */
    void RequestPath();

    /** Move step toward current waypoint */
    void MoveAlongPath(float DeltaTime);
};
