
#include "Grid/WrapperTower.h"


AWrapperTower::AWrapperTower()
{
	PrimaryActorTick.bCanEverTick = false;

	// TowerCost here!
	TowerCost = 100;

	// Spawn real tower in constructor (had to do it cus of messy code in TestTower...)
	if (HasAnyFlags(RF_ClassDefaultObject) == false && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.bDeferConstruction = true; // finish construction manually

		RealTower = GetWorld()->SpawnActor<ATestTower>(FVector::ZeroVector, FRotator::ZeroRotator, Params);

		if (RealTower)
		{
			// Finish construction + begin play so all beginplay within the TestTower gets runned
			RealTower->FinishSpawning(FTransform(GetActorRotation(), GetActorLocation()));

			// Snap to this class
			RealTower->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			// Give buildmanager something to see in preview mode
			if (RealTower->BPTowerMesh && RealTower->BPTowerMesh->GetStaticMesh())
			{
				TowerMesh = RealTower->BPTowerMesh; // makes preview visible!
			}
			else if (RealTower->GetRootComponent() && RealTower->GetRootComponent()->IsA<UStaticMeshComponent>())
			{
				TowerMesh = Cast<UStaticMeshComponent>(RealTower->GetRootComponent());
			}
		}
	}
}

void AWrapperTower::SetPreviewMode(bool bIsPreview)
{
	// base class does scale + material
	Super::SetPreviewMode(bIsPreview);

	if (RealTower)
	{
		const FVector Scale = bIsPreview ? FVector(1.05f) : FVector(1.0f);
		RealTower->SetActorScale3D(Scale);

		// Make tower translucent in preview
		if (bIsPreview && RealTower->BPTowerMesh)
		{
			RealTower->BPTowerMesh->SetScalarParameterValueOnMaterials(FName("IsPreview"), 1.0f);
		}
		else if (RealTower->BPTowerMesh)
		{
			RealTower->BPTowerMesh->SetScalarParameterValueOnMaterials(FName("IsPreview"), 0.0f);
		}
	}
}

void AWrapperTower::Destroyed()
{
	// Make sure tower dies with this class
	if (RealTower && RealTower->IsValidLowLevel())
	{
		RealTower->Destroy();
		RealTower = nullptr;
	}

	Super::Destroyed();
}

void AWrapperTower::BeginPlay()
{
	Super::BeginPlay();

}
