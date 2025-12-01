
#include "Grid/Tower.h"

ATower::ATower()
{
	PrimaryActorTick.bCanEverTick = false;

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = TowerMesh;

	// Collision
	TowerMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TowerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ATower::BeginPlay()
{
	Super::BeginPlay();

}

void ATower::SetPreviewMode(bool bIsPreview)
{
	if (!TowerMesh) return;

	// c++ fallback
	FVector Scale = bIsPreview ? FVector(1.05f) : FVector(1.0f);
	SetActorScale3D(Scale);
	TowerMesh->SetScalarParameterValueOnMaterials(FName("IsPreview"), bIsPreview ? 1.0f : 0.0f);

	// hide
	SetActorHiddenInGame(false);
}
