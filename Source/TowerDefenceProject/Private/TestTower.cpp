// Fill out your copyright notice in the Description page of Project Settings.


#include "TestTower.h"
#include "StandardProjectile.h"
#include "TimerManager.h"
#include "ProjectilePool.h"
#include "TowerTreeManager.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATestTower::ATestTower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ATestTower::Test() //REMOVE
{
	TowerTreeManager->GoToNode(this, CurrentNode->Children[1]);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Input Worked"));
}

void ATestTower::GetUpgradeUI(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Mesh Clicked!"));
	}
}

// Called when the game starts or when spawned
void ATestTower::BeginPlay()
{
	Super::BeginPlay();

	if (BPTowerMesh)
	{
		RootComponent = BPTowerMesh;
		BPTowerMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		BPTowerMesh->SetGenerateOverlapEvents(true);
		BPTowerMesh->SetCollisionResponseToAllChannels(ECR_Block);
		BPTowerMesh->OnClicked.AddDynamic(this, &ATestTower::GetUpgradeUI);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BPTowerMesh hasn't loaded yet"));
	}
	
	ResetTimer();
	if (Cast<AProjectilePool>(UGameplayStatics::GetActorOfClass(GetWorld(), AProjectilePool::StaticClass())))
	{
		ProjectilePool = Cast<AProjectilePool>(UGameplayStatics::GetActorOfClass(GetWorld(), AProjectilePool::StaticClass()));
	}
	else
	{
		ProjectilePool = GetWorld()->SpawnActor<AProjectilePool>(FVector(0, 0, -300), FRotator(0, 0, 0));
		UE_LOG(LogTemp, Log, TEXT("ProjectilePool created"));
	}
	
	CurrentNode = RootNodeRef.LoadSynchronous();
	
	if (Cast<ATowerTreeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATowerTreeManager::StaticClass())))
	{
		TowerTreeManager = Cast<ATowerTreeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATowerTreeManager::StaticClass()));
	}
	else
	{
		TowerTreeManager = GetWorld()->SpawnActor<ATowerTreeManager>(FVector(0, 0, -300), FRotator(0, 0, 0));
		UE_LOG(LogTemp, Log, TEXT("TowerTreeManager created"));
	}

	
}

// Called every frame
void ATestTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATestTower::Fire()
{
	if (ProjectilePool->ProjectilePool.IsEmpty())
	{
		AStandardProjectile* NewProjectile = GetWorld()->SpawnActor<AStandardProjectile>(GetActorLocation(), GetActorRotation());
		NewProjectile->ProjectilePool = ProjectilePool;
		NewProjectile->Enable(this, FRotator (0, 0, 0));
	}
	else
	{
		AStandardProjectile* Temp = ProjectilePool->ProjectilePool.Pop();
		Temp->Enable(this, FRotator (0, 0, 0));
	}
}

void ATestTower::ResetTimer()
{
	GetWorldTimerManager().SetTimer(FireRateHandle, this, &ATestTower::Fire, FireRate, true);
}
