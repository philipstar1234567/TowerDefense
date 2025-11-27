#include "WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "Grid/GridManager.h"
#include "EnemyBase.h"
#include "EnemyHandler.h"
#include "Engine/World.h"
#include "TimerManager.h"

AWaveManager::AWaveManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentWaveIndex = -1;
}

void AWaveManager::BeginPlay()
{
    Super::BeginPlay();

    if (!GridManager)
        GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

    if (!EnemyHandler)
        EnemyHandler = Cast<AEnemyHandler>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyHandler::StaticClass()));

    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("WaveManager: Missing GridManager!"));
        return;
    }
    if (!EnemyHandler)
    {
        UE_LOG(LogTemp, Error, TEXT("WaveManager: Missing EnemyHandler!"));
        return;
    }

    // auto-start first wave if configured
    if (Waves.Num() > 0)
    {
        StartWave(0);
    }
}

void AWaveManager::StartWave(int32 WaveIndex)
{
    if (!Waves.IsValidIndex(WaveIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Invalid wave index %d"), WaveIndex);
        return;
    }

    FVector SpawnWorld, TargetWorld;
    if (!GetTileWorldPositions(SpawnWorld, TargetWorld))
    {
        UE_LOG(LogTemp, Error, TEXT("WaveManager: Invalid spawn/target tiles (%d,%d)->(%d,%d)"), SpawnTile.X, SpawnTile.Y, TargetTile.X, TargetTile.Y);
        return;
    }

    CurrentWaveIndex = WaveIndex;
    EnemiesSpawnedThisWave = 0;
    EnemiesAlive = 0;

    const float Interval = FMath::Max(0.05f, Waves[WaveIndex].SpawnInterval);
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AWaveManager::SpawnNextEnemy, Interval, true);

    UE_LOG(LogTemp, Log, TEXT("WaveManager: Started wave %d from (%d,%d) to (%d,%d)"), WaveIndex, SpawnTile.X, SpawnTile.Y, TargetTile.X, TargetTile.Y);
}

void AWaveManager::SpawnNextEnemy()
{
    if (!Waves.IsValidIndex(CurrentWaveIndex)) return;
    const FEnemyWaveData& Wave = Waves[CurrentWaveIndex];

    if (EnemiesSpawnedThisWave >= Wave.EnemyCount)
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        if (EnemiesAlive <= 0) EndWave();
        return;
    }

    FVector SpawnWorld, TargetWorld;
    if (!GetTileWorldPositions(SpawnWorld, TargetWorld)) return;

    if (!Wave.EnemyClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Wave %d missing EnemyClass"), CurrentWaveIndex);
        EnemiesSpawnedThisWave++;
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // spawn above tile center to avoid collision with tile mesh
    FVector SpawnLoc = SpawnWorld + FVector(0.f, 0.f, 50.f);

    AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(Wave.EnemyClass, SpawnLoc, FRotator::ZeroRotator, Params);
    if (NewEnemy)
    {
        NewEnemy->InitializeEnemy(EnemyHandler, TargetWorld);
        NewEnemy->OnDestroyed.AddDynamic(this, &AWaveManager::OnEnemyDestroyed);
        EnemiesSpawnedThisWave++;
        EnemiesAlive++;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Failed to spawn enemy actor."));
    }
}

void AWaveManager::OnEnemyDestroyed(AActor* DestroyedActor)
{
    EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);
    if (Waves.IsValidIndex(CurrentWaveIndex))
    {
        if (EnemiesSpawnedThisWave >= Waves[CurrentWaveIndex].EnemyCount && EnemiesAlive <= 0)
        {
            EndWave();
        }
    }
}

void AWaveManager::EndWave()
{
    UE_LOG(LogTemp, Log, TEXT("WaveManager: Wave %d complete."), CurrentWaveIndex);
    CurrentWaveIndex = -1;
    EnemiesSpawnedThisWave = 0;
    EnemiesAlive = 0;
}

bool AWaveManager::GetTileWorldPositions(FVector& OutSpawnWorld, FVector& OutTargetWorld) const
{
    if (!GridManager) return false;
    if (!GridManager->IsValidTile(SpawnTile.X, SpawnTile.Y)) return false;
    if (!GridManager->IsValidTile(TargetTile.X, TargetTile.Y)) return false;

    OutSpawnWorld = GridManager->GetTileWorldLocation(SpawnTile.X, SpawnTile.Y);
    OutTargetWorld = GridManager->GetTileWorldLocation(TargetTile.X, TargetTile.Y);
    return true;
}
