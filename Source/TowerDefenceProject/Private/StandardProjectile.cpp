// Fill out your copyright notice in the Description page of Project Settings.


#include "StandardProjectile.h"

#include "EnemyBase.h"
#include "TimerManager.h"
#include "ProjectilePool.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerResourceState.h"

// Sets default values
AStandardProjectile::AStandardProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Creates the collision component
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(Size);
	CollisionComp->SetCollisionProfileName(TEXT("StandardProjectile"));
	RootComponent = CollisionComp;

	// Creates the component responsible for movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = InitialSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->Friction = 0.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// Gets the mesh and material from the Projectiles folder
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(CollisionComp);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Game/Philip/Projectiles/Shape_Sphere.Shape_Sphere"));
	StaticMeshComp->SetStaticMesh(SphereMesh.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Philip/Projectiles/M_StandardProjectile.M_StandardProjectile"));
	StaticMeshComp->SetMaterial(0, Material.Object);
	StaticMeshComp->AddRelativeLocation(FVector(0, 0, -100.0f)); //This line and the following line compensate for size differences between the mesh and the collision component
	StaticMeshComp->SetWorldScale3D(FVector(2, 2, 2));

	CollisionComp->SetWorldScale3D(FVector(0.25f, 0.25f, 0.25f));
}

void AStandardProjectile::OnHit(UPrimitiveComponent* EventGenerator, AActor* FoundActor, UPrimitiveComponent* FoundComp, int32 FoundBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AEnemyBase>(FoundActor))
	{
		AEnemyBase* FoundEnemy = Cast<AEnemyBase>(FoundActor);
		FoundEnemy->ApplyDamage(Strength);
		Disable(); // Remove projectile after hit
	}
}

// Called when the game starts or when spawned
void AStandardProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AStandardProjectile::OnHit); //This line didn't work in constructor as not everything had loaded yet, so it is here
}

// Called every frame
void AStandardProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AStandardProjectile::Disable()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorLocation(ProjectilePool->GetActorLocation());
	SetActorRotation(ProjectilePool->GetActorRotation());
	GetWorldTimerManager().ClearTimer(ProjectileLifespanHandle);
	ProjectilePool->ProjectilePool.Push(this);
}

void AStandardProjectile::Enable(ATestTower* SpawnTower, FVector MovementDirectionIn)
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorLocation(SpawnTower->BPSphere->GetComponentLocation());
	Strength = SpawnTower->Strength;
	ProjectileMovement->Velocity = MovementDirectionIn.GetSafeNormal() * ProjectileMovement->InitialSpeed;
	GetWorldTimerManager().SetTimer(ProjectileLifespanHandle, this, &AStandardProjectile::Disable, ProjectileLifespan, false);
}
