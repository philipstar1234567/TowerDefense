#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveManager.generated.h"

class AGridManager;
class AEnemyHandler;
class AEnemyBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompletedDelegate);

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
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAllWavesCompletedDelegate OnAllWavesCompleted;

    UFUNCTION(BlueprintCallable, Category = "Waves")
    void StartWave(int32 WaveIndex);

    UFUNCTION()
    void TryFindGridManager();

    UFUNCTION()
    void TryFindPlayerResourceState();

    UFUNCTION()
    void InitializeWaveManager();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AGridManager* GridManager = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    APlayerResourceState* PlayerResourceState = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AEnemyHandler* EnemyHandler = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
    TArray<FEnemyWaveData> Waves;

    FTimerHandle TryFindGridManagerHandle;
    FTimerHandle TryFindPlayerResourceStateHandle;

private:
    int32 CurrentWaveIndex = 0;
    int32 EnemiesSpawnedThisWave = 0;
    int32 EnemiesAlive = 0;

    FTimerHandle SpawnTimerHandle;

    void SpawnNextEnemy();
    UFUNCTION()
    void OnEnemyDestroyed(AActor* DestroyedActor);
    void EndWave();

    bool GetWorldSpawnAndGoal(FVector& OutSpawn, FVector& OutGoal) const;
};
