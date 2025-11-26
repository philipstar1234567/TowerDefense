#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "WaveManager.generated.h"

class AGridManager;
class AEnemyBase;
class AEnemyHandler;

USTRUCT(BlueprintType)
struct FEnemyWaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    TSubclassOf<AEnemyBase> EnemyClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 EnemyCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float SpawnInterval = 1.0f;
};

UCLASS()
class TOWERDEFENCEPROJECT_API AWaveManager : public AActor
{
    GENERATED_BODY()

public:
    AWaveManager();

    virtual void BeginPlay() override;

    /** Start a wave by index */
    UFUNCTION(BlueprintCallable, Category = "Waves")
    void StartWave(int32 WaveIndex);

protected:
    // ---- References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AGridManager* GridManager = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AEnemyHandler* EnemyHandler = nullptr;

    // ---- Spawn / Target positions ----
    /** Grid coordinate to spawn enemies from (X,Y tile index) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    FIntPoint SpawnTile = FIntPoint(0, 0);

    /** Grid coordinate enemies try to reach */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    FIntPoint TargetTile = FIntPoint(9, 9);

    // ---- Waves ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
    TArray<FEnemyWaveData> Waves;

    int32 CurrentWaveIndex;
    int32 EnemiesSpawnedThisWave;
    int32 EnemiesAlive;
    FTimerHandle SpawnTimerHandle;

    void SpawnNextEnemy();
    UFUNCTION()
    void OnEnemyDestroyed(AActor* DestroyedActor);
    void EndWave();

    /** Converts SpawnTile and TargetTile to world positions using the GridManager */
    bool GetTileWorldPositions(FVector& OutSpawnWorld, FVector& OutTargetWorld) const;
};
