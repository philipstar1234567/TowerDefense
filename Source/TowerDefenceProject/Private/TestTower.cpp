// Fill out your copyright notice in the Description page of Project Settings.


#include "TestTower.h"
#include "DrawDebugHelpers.h"
#include "AudioDevice.h"
#include "EnemyBase.h"
#include "StandardProjectile.h"
#include "Player/TopDownPawn.h"
#include "TimerManager.h"
#include "UpgradeTreeUI.h"
#include "ProjectilePool.h"
#include "TowerTreeManager.h"
#include "Components/SphereComponent.h"
#include "Grid/BuildManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATestTower::ATestTower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// Creates Enemy Detector
	EnemyDetector = CreateDefaultSubobject<USphereComponent>(TEXT("EnemyDetector"));
	EnemyDetector->InitSphereRadius(Range);
	RootComponent = EnemyDetector;
	EnemyDetector->SetCollisionProfileName(TEXT("Tower"));
	EnemyDetector->SetMobility(EComponentMobility::Movable);
	EnemyDetector->SetGenerateOverlapEvents(true);
	
	// Creates RangeMesh
	RangeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RangeMesh"));
	RangeMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Game/Philip/Towers/TestTower/Shape_Sphere.Shape_Sphere"));
	RangeMesh->SetStaticMesh(SphereMesh.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Philip/Towers/TestTower/M_Range.M_Range"));
	RangeMesh->SetMaterial(0, Material.Object);
}

void ATestTower::SetPreviewMode(bool bIsPreview)
{
	UE_LOG(LogTemp, Log, TEXT("ATestTower::SetPreviewMode ran succesfully"));
}


void ATestTower::GetUpgradeUI(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
{
	if (TowerTreeManager->bIsInUpgradeMenu == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0); // Gets player controller
		if (!PC) return;

		if (!UpgradeTreeUIClass)
		{
			UE_LOG(LogTemp, Log, TEXT("TestTower: UpgradeTreeUIClass is nullptr"));
			return;
		}

		if (TopDownPawn->CurrentMode == EGameMode::None) //Doesn't get UI if the player is in build or delete mode
		{
			UE_LOG(LogTemp, Log, TEXT("TestTower: Debug Check"));

			UpgradeTreeUIInstance = CreateWidget<UUpgradeTreeUI>(PC, UpgradeTreeUIClass);
			UpgradeTreeUIInstance->Tower = this;
			UpgradeTreeUIInstance->SetIsFocusable(true);
			UpgradeTreeUIInstance->AddToViewport();
			TowerTreeManager->bIsInUpgradeMenu = true;
			RangeMesh->SetVisibility(true);
		}
	}
}

void ATestTower::OnEnemyFound(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AEnemyBase>(FoundActor))
	{
		EnemyArray.Add(Cast<AEnemyBase>(FoundActor));
	}
}

void ATestTower::OnEnemyLost(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex)
{
	if (Cast<AEnemyBase>(FoundActor))
	{
		EnemyArray.Remove(Cast<AEnemyBase>(FoundActor));
	}
}

// Called when the game starts or when spawned
void ATestTower::BeginPlay()
{
	Super::BeginPlay();

	// Gets cube part of tower from blueprint and sets up click event
	if (BPCube)
	{
		BPCube->SetupAttachment(RootComponent);
		BPCube->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		BPCube->SetGenerateOverlapEvents(true);
		BPCube->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		BPCube->OnClicked.AddDynamic(this, &ATestTower::GetUpgradeUI);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BPCube hasn't loaded yet"));
	}
	
	// Gets cube part of tower from blueprint and sets up click event
	if (BPSphere)
	{
		BPSphere->SetupAttachment(RootComponent);
		BPSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		BPSphere->SetGenerateOverlapEvents(true);
		BPSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		BPSphere->OnClicked.AddDynamic(this, &ATestTower::GetUpgradeUI);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BPSphere hasn't loaded yet"));
	}
	
	ResetTimer(); //Sets up firerate timer
	
	// Get reference to AProjectilePool, create one if it doesn't exist
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
	
	// Get reference to ATowerTreeManager, create one if it doesn't exist
	if (Cast<ATowerTreeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATowerTreeManager::StaticClass())))
	{
		TowerTreeManager = Cast<ATowerTreeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATowerTreeManager::StaticClass()));
	}
	else
	{
		TowerTreeManager = GetWorld()->SpawnActor<ATowerTreeManager>(FVector(0, 0, -300), FRotator(0, 0, 0));
		UE_LOG(LogTemp, Log, TEXT("TowerTreeManager created"));
	}
	
	// Get reference to ATopDownPawn, game won't start without this so we assume it exists
	if (Cast<ATopDownPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATopDownPawn::StaticClass())))
	{
		TopDownPawn = Cast<ATopDownPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATopDownPawn::StaticClass()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TopDownPawn could not be found!"));
	}
	
	//Last few lines here are things that the constructor wasn't able to do as the components weren't loaded.
	EnemyDetector->OnComponentBeginOverlap.AddDynamic(this, &ATestTower::OnEnemyFound);
	EnemyDetector->OnComponentEndOverlap.AddDynamic(this, &ATestTower::OnEnemyLost);
	FixRangeMeshSize();
	RangeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RangeMesh->SetVisibility(true);
	
}

// Called every frame
void ATestTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATestTower::Fire()
{
	if (bIsPlaced == true && EnemyArray.IsEmpty() == false) //Checks if the tower is placed and the tower has an enemy in range
	{
		FVector EnemyLocation = EnemyArray[0]->CollisionComp->GetComponentLocation();
		FVector AimDirection = EnemyLocation - BPSphere->GetComponentLocation(); // Gets direction to aim in
		
		if (ProjectilePool->ProjectilePool.IsEmpty()) // If the pool is empty, spawn a completely new projectile, otherwise pop one from the pool
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

void ATestTower::FixRangeMeshSize()
{
	RangeMesh->SetWorldScale3D(FVector(Range/50));
	RangeMesh->SetRelativeLocation(FVector(0, 0, -Range)); //Calculations are done based on range so it will scale correctly
}
