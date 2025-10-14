
#include "Player/TopDownPawn.h"

// Constructor
ATopDownPawn::ATopDownPawn()
{
	// Initialize Components
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot; // Set as root

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->AddLocalRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 0.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetProjectionMode(ECameraProjectionMode::Perspective);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	FloatingPawnMovement->MaxSpeed = 1200.0f; // Can add this into a settings for movementspeed

	// Initialize Variables - Defaults
	ZoomSpeed = 100.0f;
	ZoomLevel = 0.0f;
	YawRotationSpeed = 2.0f; // Value limit 1 - 10
	BaseCameraAngle = -60.0f;
	TargetPitch = BaseCameraAngle;
	bCameraRotationActive = false;
	bCameraInterpolationActive = false;
	bGodViewEnabled = false;
	bBuildModeEnabled = false;
	bInvertedScrollDirection = true;

	// default thingy, dont know if needed
	PrimaryActorTick.bCanEverTick = true;
}

void ATopDownPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// Spawn GridManager
	if (GridManagerClass)
	{
		FVector SpawnLocation = FVector(0.f, 0.f, 0.f);
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GridManagerRef = GetWorld()->SpawnActor<AGridManager>(
			GridManagerClass, SpawnLocation, SpawnRotation, SpawnParams
		);

		if (!GridManagerRef)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn GridManager!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GridManagerClass is not set in TopDownPawn!"));
	}

	// Spawn BuildManager
	if (BuildManagerClass)
	{
		FVector SpawnLocation = FVector(0.f, 0.f, 0.f);
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		BuildManagerRef = GetWorld()->SpawnActor<ABuildManager>(
			BuildManagerClass, SpawnLocation, SpawnRotation, SpawnParams
		);

		if (!BuildManagerRef)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn BuildManager!"));
		}
	}

	if (BuildManagerRef && GridManagerRef)
	{
		BuildManagerRef->SetGridManager(GridManagerRef); // Giving the reference
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		// just getting the center of the viewport for later
		int32 ViewportX, ViewportY;
		PC->GetViewportSize(ViewportX, ViewportY);
		ViewportCenter = FVector2D(ViewportX / 2, ViewportY / 2);

		PC->bShowMouseCursor = true;

		// Get the local player subsystem
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

			if (Subsystem && DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TopDownPawn: PlayerController not found!"));
		return;
	}
}

void ATopDownPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Camera interpolation logic
	if (bCameraInterpolationActive && SpringArm)
	{
		// Get current rotation
		FRotator CurrentRot = SpringArm->GetRelativeRotation();
		FRotator TargetRot(TargetPitch, CurrentRot.Yaw, CurrentRot.Roll);

		// Interpolate rotation
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 9.f);
		SpringArm->SetRelativeRotation(NewRot);

		// stop interpolation and snap to target if close enough
		if (FMath::IsNearlyEqual(NewRot.Pitch, TargetPitch, 0.05f))
		{
			SpringArm->SetRelativeRotation(TargetRot); // snap exactly
			bCameraInterpolationActive = false;
		}
	}
}

void ATopDownPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Mouse - L
		EnhancedInput->BindAction(IA_LeftMouseB, ETriggerEvent::Triggered, this, &ATopDownPawn::OnLeftMousePressed);

		// Mouse - R
		EnhancedInput->BindAction(IA_RightMouseB, ETriggerEvent::Started, this, &ATopDownPawn::OnRightMousePressed);
		EnhancedInput->BindAction(IA_RightMouseB, ETriggerEvent::Completed, this, &ATopDownPawn::OnRightMouseReleased);

		// Movement - W A S D
		EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ATopDownPawn::Move);

		// MouseCamera - MouseX
		EnhancedInput->BindAction(IA_MouseX, ETriggerEvent::Triggered, this, &ATopDownPawn::RotateCamera);

		// Toggles, View and mode - C B
		EnhancedInput->BindAction(IA_ToggleView, ETriggerEvent::Triggered, this, &ATopDownPawn::ToggleView);
		EnhancedInput->BindAction(IA_ToggleBuildMode, ETriggerEvent::Triggered, this, &ATopDownPawn::ToggleBuildMode);
		
		// Zoom - MouseWheel
		EnhancedInput->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &ATopDownPawn::ZoomCamera);
	}
}

void ATopDownPawn::OnLeftMousePressed()
{
	UE_LOG(LogTemp, Log, TEXT("Left Mouse Pressed"));
	if (bBuildModeEnabled && BuildManagerRef)
	{
		BuildManagerRef->TryPlaceTower();
	}
}

void ATopDownPawn::OnRightMousePressed()
{
	if (!bCameraInterpolationActive)
	{
		bCameraRotationActive = true;

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->bShowMouseCursor = false;

			BuildManagerRef->OnPlayerRotating(true);

			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
		}
	}
}

void ATopDownPawn::OnRightMouseReleased()
{
	bCameraRotationActive = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Reset mouse to center
		//PC->SetMouseLocation(ViewportCenter.X, ViewportCenter.Y);

		PC->bShowMouseCursor = true;

		BuildManagerRef->OnPlayerRotating(false);

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
}

void ATopDownPawn::Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();

	// Cancel input if it's neutral or balanced
	if (FMath::IsNearlyZero(Input.X) && FMath::IsNearlyZero(Input.Y))
		return;
	
	// Get the SpringArm rotation to orient movement with the camera
	FRotator CameraRotation = SpringArm->GetComponentRotation();
	FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f); // Only yaw affects movement

	// Create forward and right vectors
	FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// Combine into world direction
	FVector MoveDirection = (Forward * Input.Y + Right * Input.X).GetSafeNormal();

	AddMovementInput(MoveDirection, 1.0f);
}

void ATopDownPawn::RotateCamera(const FInputActionValue& Value)
{
	if (!bCameraRotationActive || !SpringArm)
		return;

	// Calculate new yaw
	float DeltaX = Value.Get<float>();

	FRotator CurrentRot = SpringArm->GetRelativeRotation();

	float NewYaw = CurrentRot.Yaw + DeltaX * YawRotationSpeed;
	NewYaw = FRotator::NormalizeAxis(NewYaw);

	FRotator NewRot(CurrentRot.Pitch, NewYaw, CurrentRot.Roll);

	SpringArm->SetRelativeRotation(NewRot);
}

void ATopDownPawn::ToggleView()
{
	if (bCameraRotationActive)
	{
		bCameraRotationActive = false;
	}

	if (!bCameraInterpolationActive)
	{
		UE_LOG(LogTemp, Log, TEXT("Toggle View: %s"), bGodViewEnabled ? TEXT("Normal View") : TEXT("God View"));
		bGodViewEnabled = !bGodViewEnabled;
		TargetPitch = bGodViewEnabled ? -90.f : BaseCameraAngle;
		bCameraInterpolationActive = true; // Activates on tick and needs to be last
	}
}

void ATopDownPawn::ToggleBuildMode()
{
	bBuildModeEnabled = !bBuildModeEnabled;

	if (GridManagerRef)
	{
		for (int32 X = 0; X < GridManagerRef->GridWidth; ++X)
		{
			for (int32 Y = 0; Y < GridManagerRef->GridHeight; ++Y)
			{
				FTileData& Tile = GridManagerRef->TileGrid[X][Y];
				ETileState NewState = bBuildModeEnabled
					? (Tile.bIsOccupied ? ETileState::Occupied : ETileState::Buildable)
					: ETileState::Default;

				GridManagerRef->SetTileState(X, Y, NewState);
			}
		}
	}
	BuildManagerRef->SetBuildModeActive(bBuildModeEnabled);
}

void ATopDownPawn::ZoomCamera(const FInputActionValue& Value)
{
	if (!SpringArm)
		return;

	float InputValue = Value.Get<float>();

	if (bInvertedScrollDirection)
		InputValue = InputValue * -1;

	// Adjust Zoom
	ZoomLevel += InputValue * ZoomSpeed;

	// Clamp Zoom
	ZoomLevel = FMath::Clamp(ZoomLevel, 0, 1500);

	// Apply Zoom to SpringArm
	SpringArm->TargetArmLength = ZoomLevel;
}
