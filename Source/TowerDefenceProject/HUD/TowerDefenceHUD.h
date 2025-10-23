
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUD/TDMainHUD.h"
#include "Player/PlayerResourceState.h"
#include "Grid/BuildManager.h"
#include "TowerDefenceHUD.generated.h"

UCLASS()
class TOWERDEFENCEPROJECT_API ATowerDefenceHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATowerDefenceHUD();

	virtual void BeginPlay() override;

	UFUNCTION()
	void UpdateResources(int32 Gold, int32 Health);

	UFUNCTION()
	void UpdateMode(EGameMode NewMode);

	UFUNCTION()
	void InitializeBindings(APlayerResourceState* InPlayerResources, ABuildManager* InBuildManager);

protected:
	// Widget class to spawn (set in editor)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTDMainHUD> MainHUDWidgetClass;

	UPROPERTY()
	UTDMainHUD* MainHUDInstance;

	UPROPERTY()
	APlayerResourceState* PlayerResources;

	UPROPERTY()
	ABuildManager* BuildManagerRef;

};
