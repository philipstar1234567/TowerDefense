#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Grid/GridManager.h"
#include "EnemyBase.generated.h"

UCLASS()
class TOWERDEFENCEPROJECT_API AEnemyBase : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // Called by spawner or wave controller
    UFUNCTION(BlueprintCallable)
    void InitializeEnemy(AGridManager* InGridManager, const FVector& InTargetLocation);

protected:
    // Grid and path data
    UPROPERTY()
    AGridManager* GridManager;

    UPROPERTY()
    TArray<FVector> PathPoints;

    int32 CurrentPathIndex;

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float PathTolerance = 25.f; // Distance to next point before switching

    FVector TargetLocation;

    void MoveAlongPath(float DeltaTime);
    void OnPathComplete();
};
