#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HUD/EndGameWidget.h"
#include "WaveManager.generated.h"

class AGridManager;
class AEnemyHandler;
class AEnemyBase;
class APlayerResourceState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompletedDelegate);

/**
 * @struct FEnemyWaveData
 * @brief Defines the configuration for a single enemy wave.
 */
USTRUCT(BlueprintType)
struct FEnemyWaveData
{
    GENERATED_BODY()

    /** The enemy class to spawn for this wave. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    TSubclassOf<AEnemyBase> EnemyClass = nullptr;

    /** Total number of enemies to spawn during this wave. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 EnemyCount = 5;

    /** Time interval (seconds) between individual enemy spawns. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float SpawnInterval = 1.0f;
};

/**
 * @class AWaveManager
 * @brief Handles the spawning, progression, and completion of enemy waves.
 *
 * This actor:
 * - Spawns enemies based on wave configurations.
 * - Tracks active enemies and wave completion.
 * - Broadcasts when all waves are finished.
 * - Automatically locates GridManager, ResourceState, and EnemyHandler if not assigned.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API AWaveManager : public AActor
{
    GENERATED_BODY()

public:
    /** Constructor. */
    AWaveManager();

    /** Initializes the wave manager and begins periodic reference lookup if needed. */
    virtual void BeginPlay() override;

    /** Widget class for the end-game UI. */
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UEndGameWidget> EndGameWidgetClass;

    /** Fired when all configured waves have finished spawning and all enemies are dead. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAllWavesCompletedDelegate OnAllWavesCompleted;

    /**
     * @brief Starts the specified wave.
     * @param WaveIndex Index of the wave in the Waves array.
     */
    UFUNCTION(BlueprintCallable, Category = "Waves")
    void StartWave(int32 WaveIndex);

    /** Attempts to locate a GridManager in the world. */
    UFUNCTION()
    void TryFindGridManager();

    /** Attempts to locate the player's resource state object. */
    UFUNCTION()
    void TryFindPlayerResourceState();

    /** Called after references are resolved. Prepares internal state. */
    UFUNCTION()
    void InitializeWaveManager();

protected:

    /** Reference to the grid manager providing spawn/goal tiles. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AGridManager* GridManager = nullptr;

    /** Reference to the player's resource and health state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    APlayerResourceState* PlayerResourceState = nullptr;

    /** Reference to the centralized enemy handler used for pathfinding. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AEnemyHandler* EnemyHandler = nullptr;

    /** List of all defined waves. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
    TArray<FEnemyWaveData> Waves;

    /** Timer responsible for finding the GridManager. */
    FTimerHandle TryFindGridManagerHandle;

    /** Timer responsible for finding the PlayerResourceState. */
    FTimerHandle TryFindPlayerResourceStateHandle;

private:

    /** Current wave index being spawned. */
    int32 CurrentWaveIndex = 0;

    /** Number of enemies spawned in the current wave. */
    int32 EnemiesSpawnedThisWave = 0;

    /** Number of currently alive enemies across the map. */
    int32 EnemiesAlive = 0;

    /** Timer for repeatedly spawning enemies during a wave. */
    FTimerHandle SpawnTimerHandle;

    /**
     * @brief Spawns the next enemy in the active wave.
     *
     * Automatically stops when the wave’s enemy count has been reached.
     */
    void SpawnNextEnemy();

    /**
     * @brief Called when an enemy is destroyed.
     * @param DestroyedActor The enemy that was removed.
     */
    UFUNCTION()
    void OnEnemyDestroyed(AActor* DestroyedActor);

    /** Ends the current wave and advances to the next one if available. */
    void EndWave();

    /**
     * @brief Retrieves spawn and goal world positions from the grid.
     * @param OutSpawn Output world position where enemies spawn.
     * @param OutGoal Output world position the enemies move toward.
     * @return True if both spawn and goal positions were successfully found.
     */
    bool GetWorldSpawnAndGoal(FVector& OutSpawn, FVector& OutGoal) const;
};
