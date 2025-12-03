



/* THIS IS NOT INCLUDED IN THE GAME */
/* DONT YOU DARE LOOK AT THIS */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Tower.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType) // Abstract: Canst spawn raw, only via subclass 
class TOWERDEFENCEPROJECT_API ATower : public AActor
{
	GENERATED_BODY()
	
public:	
	ATower();

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TowerMesh;

	// Economy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Stats|Economy")
	int32 TowerCost = 100;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tower")
	int32 GetTowerCost() const { return TowerCost; }

	// Preview hook: BP override for translucent/outline
	UFUNCTION(BlueprintCallable, Category = "Tower")
	virtual void SetPreviewMode(bool bIsPreview);

protected:
	virtual void BeginPlay() override;
};
