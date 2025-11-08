// Fill out your copyright notice in the Description page of Project Settings.


#include "TestTower.h"
#include "StandardProjectile.h"
#include "TimerManager.h"
#include "ProjectilePool.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectilePool* ProjectilePool = nullptr;

// Sets default values
ATestTower::ATestTower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATestTower::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(FireRateHandle, this, &ATestTower::Fire, 1.f, true);
	if (Cast<AProjectilePool>(UGameplayStatics::GetActorOfClass(GetWorld(), AProjectilePool::StaticClass())))
	{
		ProjectilePool = Cast<AProjectilePool>(UGameplayStatics::GetActorOfClass(GetWorld(), AProjectilePool::StaticClass()));
	}
	else
	{
		ProjectilePool = GetWorld()->SpawnActor<AProjectilePool>(FVector(0, 0, -300), FRotator(0, 0, 0));
		UE_LOG(LogTemp, Log, TEXT("ProjectilePool created"));
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
		ProjectilePool->ProjectilePool.Add(NewProjectile);
		NewProjectile->ProjectilePool = ProjectilePool;
		UE_LOG(LogTemp, Log, TEXT("Array was empty"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Array was empty, created " + NewProjectile->GetName() + " from " + this->GetName()));
		NewProjectile->Enable(this);
	}
	else
	{
		AStandardProjectile* Temp = ProjectilePool->ProjectilePool.Pop();
		Temp->Enable(this);
	}
}
