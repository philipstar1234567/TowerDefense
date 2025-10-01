
#include "TopDownCamera.h"

// Sets default values
ATopDownCamera::ATopDownCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATopDownCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATopDownCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATopDownCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

