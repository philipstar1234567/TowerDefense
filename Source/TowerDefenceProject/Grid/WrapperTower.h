
#pragma once

#include "CoreMinimal.h"
#include "Grid/Tower.h"
#include "TestTower.h"
#include "GameFramework/Actor.h"
#include "WrapperTower.generated.h"

// A bridge between my building and anothers tower, this was apparently the best way to do it...
UCLASS()
class TOWERDEFENCEPROJECT_API AWrapperTower : public ATower
{
	GENERATED_BODY()
	
public:	
	AWrapperTower();

	// Philips actial tower:
	UPROPERTY()
	ATestTower* RealTower = nullptr;

	// Forward the preview mode to my tower
	virtual void SetPreviewMode(bool bIsPreview) override;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
};
