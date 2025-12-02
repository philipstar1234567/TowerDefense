#include "EnemyBase.h"
#include "EnemyHandler.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;

    MoveSpeed = 100.f;             
    WaypointAcceptanceRadius = 4.f;
    CurrentPathIndex = 0;
    EnemyHandler = nullptr;
    
    // Creates Collision Component as a sphere and attaches it to the root
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    RootComponent = CollisionComp;
    CollisionComp->SetCollisionProfileName(TEXT("Enemy"));
    CollisionComp->SetMobility(EComponentMobility::Movable);
    CollisionComp->SetGenerateOverlapEvents(true);
    CollisionComp->InitSphereRadius(100.0f);
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    Health = MaxHealth;
}

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

void AEnemyBase::ApplyDamage(float Amount)
{
    if (Amount <= 0.f) return;

    Health -= Amount;

    if (Health <= 0.f)
    {
        Health = 0.f;

        // Broadcast death event
        OnEnemyKilled.Broadcast(GoldReward);

        Destroy();
    }
}

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

void AEnemyBase::RecalculatePath()
{
    RequestPath();
}

void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MoveAlongPath(DeltaTime);
    
    
    //FROM PHILIP, SHOWS RANGE OF HITBOX, FOR DEBUGGING
    DrawDebugSphere(
                GetWorld(),
                CollisionComp->GetComponentLocation(),
                CollisionComp->GetScaledSphereRadius(),
                24,
                FColor::Red,
                false,
                -1.f,
                0,
                2.f
            );
    
}

void AEnemyBase::MoveAlongPath(float DeltaTime)
{
    if (CurrentPath.Num() == 0 || CurrentPathIndex >= CurrentPath.Num())
    {
        return; // no path or reached the end
    }

    FVector CurrentTarget = CurrentPath[CurrentPathIndex];
    FVector Location = GetActorLocation();

    FVector ToTarget = CurrentTarget - Location;
    float Distance = ToTarget.Size();

    if (Distance < WaypointAcceptanceRadius)
    {
        // Reached this waypoint -> move to next
        CurrentPathIndex++;

        // If we reached the last waypoint, we're at the goal
        if (CurrentPathIndex >= CurrentPath.Num())
        {
            UE_LOG(LogTemp, Log, TEXT("%s reached the goal!"), *GetName());

            OnEnemyFinished.Broadcast(GoalDamage);

            Destroy(); // remove enemy or trigger event
            return;
        }
        return;
    }

    // Move toward current target
    FVector Direction = ToTarget.GetSafeNormal();
    FVector NewLocation = Location + Direction * MoveSpeed * DeltaTime;
    SetActorLocation(NewLocation);
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (EnemyHandler)
    {
        EnemyHandler->UnregisterEnemy(this);
    }

    Super::EndPlay(EndPlayReason);
}