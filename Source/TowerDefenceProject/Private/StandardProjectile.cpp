// Fill out your copyright notice in the Description page of Project Settings.


#include "StandardProjectile.h"
#include "TimerManager.h"
#include "ProjectilePool.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AStandardProjectile::AStandardProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(100.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AStandardProjectile::OnHit);
	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = 200.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->Friction = 0.0f;
	ProjectileMovement->Velocity = Direction * InitialSpeed;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(CollisionComp);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Game/Projectiles/Shape_Sphere.Shape_Sphere"));
	StaticMeshComp->SetStaticMesh(SphereMesh.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Projectiles/M_StandardProjectile.M_StandardProjectile"));
	StaticMeshComp->SetMaterial(0, Material.Object);
	StaticMeshComp->AddRelativeLocation(FVector(0, 0, -100.0f));
	StaticMeshComp->SetWorldScale3D(FVector(2, 2, 2));

	CollisionComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
}

void AStandardProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	printf("Projectile Hit\n");
}

// Called when the game starts or when spawned
void AStandardProjectile::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(ProjectileLifespanHandle, this, &AStandardProjectile::Disable, ProjectileLifespan, false);
}

void AStandardProjectile::Spawn(ATestTower* SpawnTower)
{
	printf("Spawned\n");
}

// Called every frame
void AStandardProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AStandardProjectile::Disable()
{
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorLocation(ProjectilePool->GetActorLocation());
	SetActorRotation(ProjectilePool->GetActorRotation());
	GetWorldTimerManager().ClearTimer(ProjectileLifespanHandle);
	ProjectilePool->ProjectilePool.Push(this);
}

void AStandardProjectile::Enable(ATestTower* SpawnTower)
{
	SetActorTickEnabled(true);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorLocation(SpawnTower->GetActorLocation());
	SetActorRotation(SpawnTower->GetActorRotation());
	GetWorldTimerManager().SetTimer(ProjectileLifespanHandle, this, &AStandardProjectile::Disable, ProjectileLifespan, false);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Enabled " + this->GetName() + " from " + SpawnTower->GetName()));
}
