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

    // Periodically try to find GridManager and PlayerResourceState
    GetWorldTimerManager().SetTimer(TryFindGridManagerHandle, this, &AWaveManager::TryFindGridManager, 0.1f, true);
    GetWorldTimerManager().SetTimer(TryFindPlayerResourceStateHandle, this, &AWaveManager::TryFindPlayerResourceState, 0.1f, true);
}

/**
 * @brief Attempts to locate an AGridManager instance in the world.
 *
 * If found, InitializeWaveManager() is called and the lookup timer is stopped.
 */
void AWaveManager::TryFindGridManager()
{
    if (GridManager == nullptr)
    {
        GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(this, AGridManager::StaticClass()));

        if (GridManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("WaveManager: Found GridManager after spawn."));
            InitializeWaveManager();

            GetWorldTimerManager().ClearTimer(TryFindGridManagerHandle);
        }
    }
}

/**
 * @brief Attempts to locate the player's resource state (health, gold, etc.).
 */
void AWaveManager::TryFindPlayerResourceState()
{
    if (PlayerResourceState == nullptr)
    {
        PlayerResourceState = Cast<APlayerResourceState>(UGameplayStatics::GetActorOfClass(this, APlayerResourceState::StaticClass()));

        if (PlayerResourceState)
        {
            GetWorldTimerManager().ClearTimer(TryFindPlayerResourceStateHandle);
        }
    }
}

/**
 * @brief Initializes references and automatically starts the first wave if available.
 */
void AWaveManager::InitializeWaveManager()
{
    if (!EnemyHandler)
    {
        EnemyHandler = Cast<AEnemyHandler>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyHandler::StaticClass()));
    }

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

    // Auto-start the first wave
    if (Waves.Num() > 0)
    {
        StartWave(0);
    }
}

/**
 * @brief Begins a wave defined in the Waves array.
 *
 * @param WaveIndex Index of the wave to begin.
 */
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
        UE_LOG(LogTemp, Error, TEXT("WaveManager: Invalid Spawn/Goal positions."));
        return;
    }

    CurrentWaveIndex = WaveIndex;
    EnemiesSpawnedThisWave = 0;
    EnemiesAlive = 0;

    const float Interval = FMath::Max(0.05f, Waves[WaveIndex].SpawnInterval);
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AWaveManager::SpawnNextEnemy, Interval, true);

    UE_LOG(LogTemp, Log, TEXT("WaveManager: Started wave %d"), WaveIndex);
}

/**
 * @brief Spawns the next enemy in the active wave.
 *
 * Automatically stops spawning when the wave count is reached.
 */
void AWaveManager::SpawnNextEnemy()
{
    if (!Waves.IsValidIndex(CurrentWaveIndex)) return;

    const FEnemyWaveData& Wave = Waves[CurrentWaveIndex];

    // Stop if wave is fully spawned
    if (EnemiesSpawnedThisWave >= Wave.EnemyCount)
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        if (EnemiesAlive <= 0)
        {
            EndWave();
        }
        return;
    }

    FVector SpawnWorld, GoalWorld;
    if (!GetWorldSpawnAndGoal(SpawnWorld, GoalWorld))
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Invalid Spawn/Goal during spawn. Stopping spawn timer."));
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    if (!Wave.EnemyClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Wave %d missing EnemyClass"), CurrentWaveIndex);
        EnemiesSpawnedThisWave++;
        return;
    }

    // Spawn enemy
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(Wave.EnemyClass, SpawnWorld, FRotator::ZeroRotator, Params);

    if (NewEnemy)
    {
        NewEnemy->InitializeEnemy(EnemyHandler, GoalWorld);
        NewEnemy->OnDestroyed.AddDynamic(this, &AWaveManager::OnEnemyDestroyed);
        NewEnemy->OnEnemyFinished.AddDynamic(PlayerResourceState, &APlayerResourceState::HandleEnemyFinished);
        NewEnemy->OnEnemyKilled.AddDynamic(PlayerResourceState, &APlayerResourceState::AddGold);

        EnemiesSpawnedThisWave++;
        EnemiesAlive++;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: Failed to spawn enemy."));
    }
}

/**
 * @brief Called whenever an enemy dies or reaches the goal.
 */
void AWaveManager::OnEnemyDestroyed(AActor* DestroyedActor)
{
    EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);

    if (Waves.IsValidIndex(CurrentWaveIndex))
    {
        if (EnemiesSpawnedThisWave >= Waves[CurrentWaveIndex].EnemyCount &&
            EnemiesAlive <= 0)
        {
            EndWave();
        }
    }
}

/**
 * @brief Ends the current wave and starts the next one if available.
 */
void AWaveManager::EndWave()
{
    UE_LOG(LogTemp, Log, TEXT("WaveManager: Wave %d complete."), CurrentWaveIndex);

    CurrentWaveIndex++;
    EnemiesSpawnedThisWave = 0;
    EnemiesAlive = 0;

    // No more waves left
    if (Waves.Num() > 0 && CurrentWaveIndex >= Waves.Num())
    {
        if (PlayerResourceState->IsGameOver())
        {
            return; // Player already lost
        }

        UE_LOG(LogTemp, Log, TEXT("WaveManager: All waves completed!"));
        OnAllWavesCompleted.Broadcast();

        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (!PC || !EndGameWidgetClass) return;

        UEndGameWidget* EndGameWidget = CreateWidget<UEndGameWidget>(PC, EndGameWidgetClass);
        EndGameWidget->bWinning = true;
        EndGameWidget->SetIsFocusable(true);
        EndGameWidget->AddToViewport();

        PC->SetPause(true);
        return;
    }

    // Auto-start next wave
    StartWave(CurrentWaveIndex);
}

/**
 * @brief Retrieves world-space coordinates for the spawn and goal tiles.
 *
 * @return True if both values were valid and output variables were filled.
 */
bool AWaveManager::GetWorldSpawnAndGoal(FVector& OutSpawn, FVector& OutGoal) const
{
    if (!GridManager) return false;

    const FVector2D SpawnF = GridManager->SpawnTile;
    const FVector2D GoalF = GridManager->GoalTile;

    // Check if tiles are uninitialized
    if (SpawnF.X < 0.f || SpawnF.Y < 0.f) return false;
    if (GoalF.X < 0.f || GoalF.Y < 0.f) return false;

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
