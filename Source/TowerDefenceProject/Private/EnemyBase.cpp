#include "EnemyBase.h"
#include "EnemyHandler.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

/**
 * @brief Default constructor.
 *
 * Initializes movement variables, collision component, and
 * enables ticking. Sets up the collision sphere used for
 * detecting overlaps with towers/projectiles.
 */
AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;

    MoveSpeed = 100.f;
    WaypointAcceptanceRadius = 4.f;
    CurrentPathIndex = 0;
    EnemyHandler = nullptr;

    // Create collision component and configure it
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    RootComponent = CollisionComp;

    CollisionComp->SetCollisionProfileName(TEXT("Enemy"));
    CollisionComp->SetMobility(EComponentMobility::Movable);
    CollisionComp->SetGenerateOverlapEvents(true);
    CollisionComp->InitSphereRadius(50.0f);
}

/**
 * @brief Called when the game starts or when the enemy is spawned.
 *
 * Initializes the health value based on MaxHealth.
 */
void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
}

/**
 * @brief Initializes the enemy immediately after spawning.
 *
 * Registers the enemy with the EnemyHandler and requests an initial navigation path.
 *
 * @param InEnemyHandler Pointer to the owning EnemyHandler.
 * @param InTargetLocation World position the enemy should try to reach.
 */
void AEnemyBase::InitializeEnemy(AEnemyHandler* InEnemyHandler, const FVector& InTargetLocation)
{
    EnemyHandler = InEnemyHandler;
    TargetLocation = InTargetLocation;

    if (EnemyHandler)
    {
        EnemyHandler->RegisterEnemy(this);
    }

    RequestPath();
}

/**
 * @brief Applies damage to this enemy.
 *
 * If health is reduced to zero, broadcasts the death event (OnEnemyKilled)
 * and destroys the actor.
 *
 * @param Amount Amount of damage to apply.
 */
void AEnemyBase::ApplyDamage(float Amount)
{
    if (Amount <= 0.f)
        return;

    Health -= Amount;

    // Check for death
    if (Health <= 0.f)
    {
        Health = 0.f;

        // Notify reward observers (player gold)
        OnEnemyKilled.Broadcast(GoldReward);

        Destroy();
    }
}

/**
 * @brief Requests a navigation path from the EnemyHandler.
 *
 * Clears the existing path and starts fresh from the actor's current location.
 * Logs warnings if the handler is missing or fails to compute a path.
 */
void AEnemyBase::RequestPath()
{
    CurrentPath.Empty();
    CurrentPathIndex = 0;

    if (!EnemyHandler)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyBase %s has no EnemyHandler!"), *GetName());
        return;
    }

    const FVector StartPos = GetActorLocation();

    if (EnemyHandler->FindPath(StartPos, TargetLocation, CurrentPath))
    {
        UE_LOG(LogTemp, Log, TEXT("%s found path with %d nodes."), *GetName(), CurrentPath.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s failed to find path."), *GetName());
    }
}

/**
 * @brief Forces a path recalculation.
 *
 * Useful when a tower is placed and the navigation graph changes.
 */
void AEnemyBase::RecalculatePath()
{
    RequestPath();
}

/**
 * @brief Called every frame.
 *
 * Updates movement along the current path.
 *
 * @param DeltaTime Time elapsed since previous frame.
 */
void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MoveAlongPath(DeltaTime);
    
}

/**
 * @brief Moves the enemy along its navigation path.
 *
 * Handles reaching waypoints, path completion, and triggers goal events.
 *
 * @param DeltaTime Frame delta time.
 */
void AEnemyBase::MoveAlongPath(float DeltaTime)
{
    if (CurrentPath.Num() == 0 || CurrentPathIndex >= CurrentPath.Num())
    {
        return; // No valid path or finished path
    }

    const FVector CurrentTarget = CurrentPath[CurrentPathIndex];
    const FVector Location = GetActorLocation();
    const FVector ToTarget = CurrentTarget - Location;

    const float Distance = ToTarget.Size();

    // If close enough to waypoint, advance to next
    if (Distance < WaypointAcceptanceRadius)
    {
        CurrentPathIndex++;

        // Path completed -> reached the goal
        if (CurrentPathIndex >= CurrentPath.Num())
        {
            UE_LOG(LogTemp, Log, TEXT("%s reached the goal!"), *GetName());

            OnEnemyFinished.Broadcast(GoalDamage);

            Destroy();
            return;
        }

        return;
    }

    // Move toward current waypoint
    const FVector Direction = ToTarget.GetSafeNormal();
    const FVector NewLocation = Location + Direction * MoveSpeed * DeltaTime;

    SetActorLocation(NewLocation);
}

/**
 * @brief Called when the enemy actor is removed from the world.
 *
 * Ensures it unregisters from the EnemyHandler to avoid dangling references.
 *
 * @param EndPlayReason Reason this actor stopped playing.
 */
void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (EnemyHandler)
    {
        EnemyHandler->UnregisterEnemy(this);
    }

    Super::EndPlay(EndPlayReason);
}
