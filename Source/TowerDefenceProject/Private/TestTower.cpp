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
	PrimaryActorTick.bCanEverTick = true;
	
	EnemyDetector = CreateDefaultSubobject<USphereComponent>(TEXT("EnemyDetector"));
	EnemyDetector->InitSphereRadius(Range);
	RootComponent = EnemyDetector;
	EnemyDetector->SetCollisionProfileName(TEXT("Tower"));
	EnemyDetector->SetMobility(EComponentMobility::Movable);
	EnemyDetector->SetGenerateOverlapEvents(true);
	
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
	if (TowerTreeManager->bIsInUpgradeMenu == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (!PC) return;

		if (!UpgradeTreeUIClass)
		{
			UE_LOG(LogTemp, Log, TEXT("TestTower: UpgradeTreeUIClass is nullptr"));
			return;
		}

		if (TopDownPawn->CurrentMode == EGameMode::None)
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
	if (Cast<AEnemyBase>(FoundActor))
	{
		EnemyArray.Add(Cast<AEnemyBase>(FoundActor));
		GEngine->AddOnScreenDebugMessage(
			-1,                     // Key: -1 means add a new message each time
			5.0f,                   // Time to display in seconds
			FColor::Green,         // Text color
			FoundActor->GetName() + " In Array" // Message text
		);
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
	if (Cast<AEnemyBase>(FoundActor))
	{
		EnemyArray.Remove(Cast<AEnemyBase>(FoundActor));
		GEngine->AddOnScreenDebugMessage(
			-1,                     // Key: -1 means add a new message each time
			5.0f,                   // Time to display in seconds
			FColor::Red,         // Text color
			FoundActor->GetName() + " Out Of Array" // Message text
		);
	}
}

// Called when the game starts or when spawned
void ATestTower::BeginPlay()
{
	Super::BeginPlay();

	if (BPCube)
	{
		BPCube->SetupAttachment(RootComponent);
		BPCube->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //CHANGE FOR UI
		BPCube->SetGenerateOverlapEvents(true);
		BPCube->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		BPCube->OnClicked.AddDynamic(this, &ATestTower::GetUpgradeUI);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BPCube hasn't loaded yet"));
	}
	
	if (BPSphere)
	{
		BPSphere->SetupAttachment(RootComponent);
		BPSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //CHANGE FOR UI
		BPSphere->SetGenerateOverlapEvents(true);
		BPSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		BPSphere->OnClicked.AddDynamic(this, &ATestTower::GetUpgradeUI);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BPSphere hasn't loaded yet"));
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
	
	if (Cast<ATopDownPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATopDownPawn::StaticClass())))
	{
		TopDownPawn = Cast<ATopDownPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATopDownPawn::StaticClass()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TopDownPawn could not be found!"));
	}
	
	EnemyDetector->OnComponentBeginOverlap.AddDynamic(this, &ATestTower::OnEnemyFound);
	EnemyDetector->OnComponentEndOverlap.AddDynamic(this, &ATestTower::OnEnemyLost);
	FixRangeMeshSize();
	RangeMesh->SetVisibility(false);
	
}

// Called every frame
void ATestTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DrawDebugSphere(
			GetWorld(),
			EnemyDetector->GetComponentLocation(), // Sphere center
			Range,                               // Radius matching Range
			32,                                  // Sphere segments for smoothness
			FColor::Green,                       // Sphere color
			false,                              // Persistent (false = duration only)
			-1.f,                               // Duration (-1 = single frame)
			0,                                  // Depth priority
			2.0f                               // Thickness of the wireframe
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
		FVector AimDirection = EnemyLocation - BPSphere->GetComponentLocation();
		
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

void ATestTower::FixRangeMeshSize()
{
	RangeMesh->SetWorldScale3D(FVector(Range/50));
	RangeMesh->SetRelativeLocation(FVector(0, 0, -Range));
}
