#include "WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "Grid/GridManager.h"
#include "EnemyBase.h"
#include "EnemyHandler.h"

AWaveManager::AWaveManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentWaveIndex = -1;
    EnemiesSpawnedThisWave = 0;
    EnemiesAlive = 0;
}

void AWaveManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find references
    if (!GridManager)
        GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

    if (!EnemyHandler)
        EnemyHandler = Cast<AEnemyHandler>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyHandler::StaticClass()));

    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("WaveManager: Missing GridManager! Place one in the level."));
        return;
    }

    if (!EnemyHandler)
    {
        UE_LOG(LogTemp, Error, TEXT("WaveManager: Missing EnemyHandler! Place one in the level."));
        return;
    }

    // Optional: auto-start first wave
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
        UE_LOG(LogTemp, Error, TEXT("WaveManager: Invalid spawn or target tile! Check grid indices."));
        return;
    }

    CurrentWaveIndex = WaveIndex;
    EnemiesSpawnedThisWave = 0;
    EnemiesAlive = 0;

    const float Interval = FMath::Max(0.05f, Waves[WaveIndex].SpawnInterval);
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AWaveManager::SpawnNextEnemy, Interval, true);

    UE_LOG(LogTemp, Log, TEXT("WaveManager: Started wave %d from (%d,%d) to (%d,%d)"),
        WaveIndex, SpawnTile.X, SpawnTile.Y, TargetTile.X, TargetTile.Y);
}

void AWaveManager::SpawnNextEnemy()
{
    if (!Waves.IsValidIndex(CurrentWaveIndex)) return;
    const FEnemyWaveData& WaveData = Waves[CurrentWaveIndex];

    if (EnemiesSpawnedThisWave >= WaveData.EnemyCount)
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        if (EnemiesAlive <= 0) EndWave();
        return;
    }

    FVector SpawnWorld, TargetWorld;
    if (!GetTileWorldPositions(SpawnWorld, TargetWorld)) return;

    if (!WaveData.EnemyClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: No EnemyClass for wave %d"), CurrentWaveIndex);
        EnemiesSpawnedThisWave++;
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(
        WaveData.EnemyClass,
        SpawnWorld + FVector(0, 0, 50.f), // slightly above tile
        FRotator::ZeroRotator,
        Params
    );

    if (NewEnemy)
    {
        NewEnemy->InitializeEnemy(EnemyHandler, TargetWorld);
        NewEnemy->OnDestroyed.AddDynamic(this, &AWaveManager::OnEnemyDestroyed);
        EnemiesSpawnedThisWave++;
        EnemiesAlive++;
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
    UE_LOG(LogTemp, Log, TEXT("WaveManager: Wave %d complete!"), CurrentWaveIndex);
    CurrentWaveIndex = -1;
    EnemiesSpawnedThisWave = 0;
    EnemiesAlive = 0;
}

bool AWaveManager::GetTileWorldPositions(FVector& OutSpawnWorld, FVector& OutTargetWorld) const
{
    if (!GridManager) return false;

    if (!GridManager->TileGrid.IsValidIndex(SpawnTile.X) ||
        !GridManager->TileGrid[SpawnTile.X].IsValidIndex(SpawnTile.Y))
        return false;

    if (!GridManager->TileGrid.IsValidIndex(TargetTile.X) ||
        !GridManager->TileGrid[TargetTile.X].IsValidIndex(TargetTile.Y))
        return false;

    OutSpawnWorld = GridManager->TileGrid[SpawnTile.X][SpawnTile.Y].WorldLocation;
    OutTargetWorld = GridManager->TileGrid[TargetTile.X][TargetTile.Y].WorldLocation;
    return true;
}
