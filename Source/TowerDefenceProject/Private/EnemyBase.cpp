#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentPathIndex = 0;
}

void AEnemyBase::InitializeEnemy(AGridManager* InGridManager, const FVector& InTargetLocation)
{
    GridManager = InGridManager;
    TargetLocation = InTargetLocation;
    PathPoints.Empty();

    if (GridManager)
    {
        bool bFoundPath = GridManager->FindPath(GetActorLocation(), TargetLocation, PathPoints);

        if (!bFoundPath)
        {
            UE_LOG(LogTemp, Warning, TEXT("EnemyBase: No path found to goal!"));
        }
        else
        {
            CurrentPathIndex = 0;
        }
    }
}

void AEnemyBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (PathPoints.Num() > 0 && CurrentPathIndex < PathPoints.Num())
    {
        MoveAlongPath(DeltaSeconds);
    }
}

void AEnemyBase::MoveAlongPath(float DeltaTime)
{
    if (CurrentPathIndex >= PathPoints.Num())
        return;

    const FVector CurrentTarget = PathPoints[CurrentPathIndex];
    FVector ToTarget = CurrentTarget - GetActorLocation();
    ToTarget.Z = 0.f;

    float Distance = ToTarget.Length();
    FVector Direction = ToTarget.GetSafeNormal();

    // Move the character toward the target
    AddMovementInput(Direction, MoveSpeed * DeltaTime);

    // If you're not using CharacterMovement, use:
    // SetActorLocation(GetActorLocation() + Direction * MoveSpeed * DeltaTime, true);

    if (Distance <= PathTolerance)
    {
        CurrentPathIndex++;

        if (CurrentPathIndex >= PathPoints.Num())
        {
            OnPathComplete();
        }
    }
    DrawDebugSphere(GetWorld(), PathPoints[CurrentPathIndex], 10.f, 8, FColor::Red);
}

void AEnemyBase::OnPathComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Enemy reached target!"));
    // You can trigger damage to the player base or destroy the enemy here
}
