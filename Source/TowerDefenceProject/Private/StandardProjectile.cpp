// Fill out your copyright notice in the Description page of Project Settings.


#include "StandardProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AStandardProjectile::AStandardProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->InitSphereRadius(10.0f);
	SphereComp->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	SphereComp->OnComponentHit.AddDynamic(this, &AStandardProjectile::OnHit);
	RootComponent = SphereComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(SphereComp);
	ProjectileMovement->InitialSpeed = 100.0f;
	ProjectileMovement->MaxSpeed = 200.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->Friction = 0.0f;

}

void AStandardProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	printf("Projectile Hit\n");
}

// Called when the game starts or when spawned
void AStandardProjectile::BeginPlay()
{
	Super::BeginPlay();
	//Tower->FFireObject.AddDynamic(this, &AStandardProjectile::Spawn);
	
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

/*Yes — you’re definitely on the right track with your goal of letting ATestTower spawn any number of AStandardProjectiles. You’re correct that CreateDefaultSubobject() is only valid in constructors (typically for components), but spawning actors at runtime (like your projectile) is done a different way.

🔑 Next Step (without full implementation):

Start looking into using GetWorld()->SpawnActor<AStandardProjectile>() in your ATestTower class to dynamically spawn projectiles at runtime.

That means your projectile class doesn’t need to live inside ATestTower as a component — instead, the tower spawns instances of the projectile whenever it fires.

So ask yourself:

Where in ATestTower should the firing happen?

Do you have a TSubclassOf<AStandardProjectile> reference to choose which projectile to spawn?

Let me know if you want help structuring that spawn logic!*/