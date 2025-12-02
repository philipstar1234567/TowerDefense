#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBase.generated.h"

class AEnemyHandler;
class USphereComponent;

/**
 * @brief Delegate fired when an enemy reaches the goal.
 * @param DamageAmount Amount of damage to apply to the player's goal.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyFinishedDelegate, int32, DamageAmount);

/**
 * @brief Delegate fired when an enemy is killed.
 * @param GoldReward Amount of gold rewarded to the player.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyKilled, int32, GoldReward);

/**
 * @class AEnemyBase
 * @brief Base class for all enemy actors in the Tower Defence project.
 *
 * Handles navigation along a computed path, collision with towers/projectiles,
 * taking damage, triggering reward/penalty events, and interacting with
 * EnemyHandler for path updates.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API AEnemyBase : public AActor
{
    GENERATED_BODY()

public:

    /** @brief Constructor. Initializes default values. */
    AEnemyBase();

    /** @brief Called every frame. */
    virtual void Tick(float DeltaTime) override;

    /** @brief Called when the game starts or the enemy is spawned. */
    virtual void BeginPlay() override;

    /**
     * @brief Cleanup when enemy actor ends play.
     * @param EndPlayReason The reason the actor stopped playing.
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /**
     * @brief Initializes the enemy immediately after spawning.
     *
     * Sets the owning EnemyHandler reference and requests the initial path
     * toward @p InTargetLocation.
     *
     * @param InEnemyHandler Pointer to the EnemyHandler that owns this enemy.
     * @param InTargetLocation The world location to pathfind toward.
     */
    void InitializeEnemy(AEnemyHandler* InEnemyHandler, const FVector& InTargetLocation);

    /**
     * @brief Forces a path recalculation.
     *
     * Called by EnemyHandler when the world navigation changes
     * (e.g., a tower was placed, blocking a route).
     */
    UFUNCTION()
    void RecalculatePath();

    /** @brief Collision component detecting overlaps with towers/projectiles. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USphereComponent* CollisionComp;

    /** @brief Broadcast when the enemy reaches the end of the path. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEnemyFinishedDelegate OnEnemyFinished;

    /** @brief Broadcast when the enemy dies and rewards the player. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEnemyKilled OnEnemyKilled;

    /** @brief Damage applied to the player goal upon finishing. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoalDamage = 1;

    /** @brief Gold rewarded to the player on death. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 GoldReward = 20;

    /** @brief Maximum health of this enemy. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.f;

    /** @brief Current remaining health. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float Health;

    /**
     * @brief Applies a damage amount to the enemy.
     *
     * If health reaches zero, triggers death behavior and reward events.
     *
     * @param Amount Amount of hit point damage.
     */
    UFUNCTION(BlueprintCallable)
    void ApplyDamage(float Amount);

protected:

    /** @brief Cached current path from EnemyHandler. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    TArray<FVector> CurrentPath;

    /** @brief Index of the waypoint currently being moved toward. */
    int32 CurrentPathIndex;

    /** @brief Movement speed of the enemy (units per second). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed;

    /** @brief Distance considered "close enough" to reach a waypoint. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WaypointAcceptanceRadius;

    /** @brief Cached final target world location. */
    FVector TargetLocation;

    /** @brief Reference to the global EnemyHandler for pathfinding. */
    AEnemyHandler* EnemyHandler;

    /**
     * @brief Requests a fresh navigation path from EnemyHandler.
     */
    void RequestPath();

    /**
     * @brief Moves the enemy along its current path.
     *
     * @param DeltaTime Frame delta time.
     */
    void MoveAlongPath(float DeltaTime);
};
