
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TowerDefenceProjectGameMode.generated.h"

UCLASS(abstract)
class ATowerDefenceProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ATowerDefenceProjectGameMode();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainHudWidgetClass;

	UPROPERTY()
	UUserWidget* MainHUDInstance;
};



