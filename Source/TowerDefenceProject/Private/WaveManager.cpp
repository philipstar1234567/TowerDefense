#include "WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "Grid/GridManager.h"
#include "EnemyBase.h"
#include "EnemyHandler.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Player/PlayerResourceState.h"

AWaveManager::AWaveManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AWaveManager::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(TryFindGridManagerHandle, this, &AWaveManager::TryFindGridManager, 0.1f, true);
    GetWorldTimerManager().SetTimer(TryFindPlayerResourceStateHandle, this, &AWaveManager::TryFindPlayerResourceState, 0.1f, true);
}

void AWaveManager::TryFindGridManager()
{
    if (GridManager == nullptr)
    {
        GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(this, AGridManager::StaticClass()));

        if (GridManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("WaveManager: Found GridManager after spawn."));
            InitializeWaveManager();  // ← move your setup logic here

            // stop timer
            GetWorldTimerManager().ClearTimer(TryFindGridManagerHandle);
        }
    }
}

void AWaveManager::TryFindPlayerResourceState()
{
    if (PlayerResourceState == nullptr)
    {
        PlayerResourceState = Cast<APlayerResourceState>(UGameplayStatics::GetActorOfClass(this, APlayerResourceState::StaticClass()));

        if (PlayerResourceState)
        {
            // stop timer
            GetWorldTimerManager().ClearTimer(TryFindGridManagerHandle);
        }
    }
}

void AWaveManager::InitializeWaveManager()
{
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

    // Auto-start first wave
    if (Waves.Num() > 0)
        StartWave(0);
}

void AWaveManager::StartWave(int32 WaveIndex)
{
    if (!Waves.IsValidIndex(WaveIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Invalid wave index %d"), WaveIndex);
        return;
    }

    FVector SpawnWorld, GoalWorld;
    if (!GetWorldSpawnAndGoal(SpawnWorld, GoalWorld))
    {
        UE_LOG(LogTemp, Error, TEXT("WaveManager: GridManager SpawnTile/GoalTile are invalid or uninitialized!"));
        return;
    }

    CurrentWaveIndex = WaveIndex;
    EnemiesSpawnedThisWave = 0;
    EnemiesAlive = 0;

    const float Interval = FMath::Max(0.05f, Waves[WaveIndex].SpawnInterval);
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AWaveManager::SpawnNextEnemy, Interval, true);

    UE_LOG(LogTemp, Log, TEXT("WaveManager: Started wave %d"), WaveIndex);
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

    FVector SpawnWorld, GoalWorld;
    if (!GetWorldSpawnAndGoal(SpawnWorld, GoalWorld))
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Spawn/Goal invalid during spawn step; stopping spawn timer."));
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    if (!Wave.EnemyClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Wave %d missing EnemyClass"), CurrentWaveIndex);
        EnemiesSpawnedThisWave++;
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    FVector SpawnLoc = SpawnWorld;

    AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(Wave.EnemyClass, SpawnLoc, FRotator::ZeroRotator, Params);
    if (NewEnemy)
    {
        NewEnemy->InitializeEnemy(EnemyHandler, GoalWorld);
        NewEnemy->OnDestroyed.AddDynamic(this, &AWaveManager::OnEnemyDestroyed);
        NewEnemy->OnEnemyFinished.AddDynamic(PlayerResourceState, &APlayerResourceState::HandleEnemyFinished);
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

bool AWaveManager::GetWorldSpawnAndGoal(FVector& OutSpawn, FVector& OutGoal) const
{
    if (!GridManager) return false;

    const FVector2D SpawnF = GridManager->SpawnTile;
    const FVector2D GoalF = GridManager->GoalTile;

    // Check if uninitialized
    if (SpawnF.X < 0.f || SpawnF.Y < 0.f) return false;
    if (GoalF.X < 0.f || GoalF.Y < 0.f) return false;

    // Convert to integer tile indices (round to nearest)
    const int32 SpawnX = FMath::RoundToInt(SpawnF.X);
    const int32 SpawnY = FMath::RoundToInt(SpawnF.Y);
    const int32 GoalX = FMath::RoundToInt(GoalF.X);
    const int32 GoalY = FMath::RoundToInt(GoalF.Y);

    if (!GridManager->IsValidTile(SpawnX, SpawnY)) return false;
    if (!GridManager->IsValidTile(GoalX, GoalY)) return false;

    OutSpawn = GridManager->GetTileWorldLocation(SpawnX, SpawnY);
    OutGoal = GridManager->GetTileWorldLocation(GoalX, GoalY);
    return true;
}
