// Fill out your copyright notice in the Description page of Project Settings.


#include "TestTower.h"
#include "DrawDebugHelpers.h"
#include "AudioDevice.h"
#include "EnemyBase.h"
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
	PrimaryActorTick.bCanEverTick = true;
	
	//TowerRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TowerRoot"));
	//TowerRoot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	//TowerRoot->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	//RootComponent = TowerRoot;
	
	EnemyDetector = CreateDefaultSubobject<USphereComponent>(TEXT("EnemyDetector"));
	EnemyDetector->InitSphereRadius(Range);
	RootComponent = EnemyDetector;
	//EnemyDetector->SetCollisionObjectType(ECC_WorldDynamic);
	EnemyDetector->SetCollisionProfileName(TEXT("Tower"));
	EnemyDetector->SetGenerateOverlapEvents(true);
	//EnemyDetector->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//EnemyDetector->SetCollisionResponseToAllChannels(ECR_Ignore);
	//EnemyDetector->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	//EnemyDetector->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	EnemyDetector->OnComponentBeginOverlap.AddDynamic(this, &ATestTower::OnEnemyFound);
	EnemyDetector->OnComponentEndOverlap.AddDynamic(this, &ATestTower::OnEnemyLost);
	//Here you have to set the collision profile, and make sure the collision type matches in the enemybase class
}

void ATestTower::SetPreviewMode(bool bIsPreview)
{
	UE_LOG(LogTemp, Log, TEXT("ATestTower::SetPreviewMode ran succesfully"));
}

void ATestTower::Test() //REMOVE
{
	TowerTreeManager->GoToNode(this, CurrentNode->Children[1]);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Input Worked"));
}

/**
 * Puts the upgrade UI on screen after clicking on a tower.
 * The UI will reflect the upgrades of the tower and you can choose a new one.
 * 
 * @param ClickedComp The tower clicked on
 * @param ButtonPressed I have no fucking idea
 */
void ATestTower::GetUpgradeUI(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Mesh Clicked!"));
	}
}

/**
 * Function called when an enemy comes close enough to a tower to
 * generate an overlap event. Adds the enemy to the tower's 
 * EnemyArray. The array contains all valid targets for the tower.
 * 
 * @param EventGenerator 
 * @param FoundActor 
 * @param FoundComp 
 * @param FoundBodyIndex 
 * @param bFromSweep 
 * @param SweepResult 
 */
void ATestTower::OnEnemyFound(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(
			-1,                     // Key: -1 means add a new message each time
			5.0f,                   // Time to display in seconds
			FColor::Yellow,         // Text color
			TEXT("Overlap event generated! Enemy entered") // Message text
		);
	if (Cast<AEnemyBase>(FoundActor))
	{
		EnemyArray.Add(Cast<AEnemyBase>(FoundActor));
	}
}

/**
 * Function called when an enemy inside a tower's range
 * leaves it. Enemy gets removed from the tower's EnemyArray
 * so the tower can't shoot at it anymore.
 * 
 * @param EventGenerator 
 * @param FoundActor 
 * @param FoundComp 
 * @param FoundBodyIndex 
 */
void ATestTower::OnEnemyLost(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(
			-1,                     // Key: -1 means add a new message each time
			5.0f,                   // Time to display in seconds
			FColor::Yellow,         // Text color
			TEXT("Overlap event generated! Enemy Left") // Message text
		);
	if (Cast<AEnemyBase>(FoundActor))
	{
		EnemyArray.Remove(Cast<AEnemyBase>(FoundActor));
	}
}

// Called when the game starts or when spawned
void ATestTower::BeginPlay()
{
	Super::BeginPlay();

	if (BPTowerMesh)
	{
		//BPTowerMesh->SetupAttachment(RootComponent);
		//BPTowerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //CHANGE FOR UI
		//BPTowerMesh->SetGenerateOverlapEvents(true);
		//BPTowerMesh->SetCollisionResponseToAllChannels(ECR_Block);
		//BPTowerMesh->OnClicked.AddDynamic(this, &ATestTower::GetUpgradeUI);
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
	DrawDebugSphere(
				GetWorld(),
				EnemyDetector->GetComponentLocation(),
				EnemyDetector->GetScaledSphereRadius(),
				24,                      // smoothness
				FColor::Green,
				false,                   // not persistent, redraw each frame
				-1.f,                    // duration (-1 = one frame if persistent false)
				0,
				2.f                      // line thickness
			);
	
}

/**
 * 
 */
void ATestTower::Fire()
{
	if (bIsPlaced == true && EnemyArray.IsEmpty() == false)
	{
		FVector EnemyLocation = EnemyArray[0]->CollisionComp->GetComponentLocation();
		FVector AimDirection = EnemyLocation - this->GetActorLocation();
		
		if (ProjectilePool->ProjectilePool.IsEmpty())
		{
			AStandardProjectile* NewProjectile = GetWorld()->SpawnActor<AStandardProjectile>(GetActorLocation(), GetActorRotation());
			NewProjectile->ProjectilePool = ProjectilePool;
			NewProjectile->Enable(this, AimDirection);
		}
		else
		{
			AStandardProjectile* Temp = ProjectilePool->ProjectilePool.Pop();
			Temp->Enable(this, AimDirection);
		}	
	}
}

void ATestTower::ResetTimer()
{
	GetWorldTimerManager().SetTimer(FireRateHandle, this, &ATestTower::Fire, FireRate, true);
}
