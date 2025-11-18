
#include "Player/TopDownPawn.h"
#include "HUD/TowerDefenceHUD.h"

ATopDownPawn::ATopDownPawn()
{
	// Initialize Components
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->AddLocalRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 0.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetProjectionMode(ECameraProjectionMode::Perspective);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	FloatingPawnMovement->MaxSpeed = 1200.0f; // Can add this into a settings for movementspeed

	// Initialize Variables
	ZoomSpeed = 100.0f;
	ZoomLevel = 0.0f;
	YawRotationSpeed = 2.0f; // Value limit 1 - 10
	BaseCameraAngle = -60.0f;
	TargetPitch = BaseCameraAngle;
	bCameraRotationActive = false;
	bCameraInterpolationActive = false;
	bGodViewEnabled = false;
	CurrentMode = EGameMode::None;
	bInvertedScrollDirection = true;

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
			GridManagerClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (!GridManagerRef)
		{
			UE_LOG(LogTemp, Error, TEXT("TopDownPawn: Failed to spawn GridManager!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TopDownPawn: GridManagerClass is not set!"));
	}

	// Spawn BuildManager
	if (BuildManagerClass)
	{
		FVector SpawnLocation = FVector(0.f, 0.f, 0.f);
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		BuildManagerRef = GetWorld()->SpawnActor<ABuildManager>(
			BuildManagerClass, SpawnLocation, SpawnRotation, SpawnParams);
		
		if (!BuildManagerRef)
		{
			UE_LOG(LogTemp, Error, TEXT("TopDownPawn: Failed to spawn BuildManager!"));
		}
	}

	if (BuildManagerRef && GridManagerRef)
	{
		BuildManagerRef->SetGridManager(GridManagerRef); // Gives a reference
		BuildManagerRef->SetMode(EGameMode::None); // quick refresh ish

		// Get and pass PlayerResourceState
		APlayerState* PS = GetPlayerState();
		if (PS)
		{
			APlayerResourceState* PRS = Cast<APlayerResourceState>(PS);
			if (PRS)
			{
				BuildManagerRef->SetPlayerResourceState(PRS);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TopDownPawn: PlayerState cast to APlayerResourceState failed!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TopDowPawn: GetPlayerState() returned null!"));
		}
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		// just getting the center of the viewport for later - might be used
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
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ATowerDefenceHUD* HUD = Cast<ATowerDefenceHUD>(PC->GetHUD()))
		{
			HUD->InitializeBindings(Cast<APlayerResourceState>(GetPlayerState()), BuildManagerRef);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TopDownPawn: Could not cast to TowerDefenceHUD!"))
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TopDownPawn: No PlayerController!"))
	}
}

void ATopDownPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCameraInterpolationActive && SpringArm)
	{
		FRotator CurrentRot = SpringArm->GetRelativeRotation();
		FRotator TargetRot(TargetPitch, CurrentRot.Yaw, CurrentRot.Roll);
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 9.f);
		SpringArm->SetRelativeRotation(NewRot);

		if (FMath::IsNearlyEqual(NewRot.Pitch, TargetPitch, 0.05f))
		{
			SpringArm->SetRelativeRotation(TargetRot);
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

		// Toggles, View and mode - C B X
		EnhancedInput->BindAction(IA_ToggleView, ETriggerEvent::Triggered, this, &ATopDownPawn::ToggleView);
		EnhancedInput->BindAction(IA_ToggleBuildMode, ETriggerEvent::Triggered, this, &ATopDownPawn::ToggleBuildMode);
		EnhancedInput->BindAction(IA_ToggleDeleteMode, ETriggerEvent::Triggered, this, &ATopDownPawn::ToggleDeleteMode);
		
		// Zoom - MouseWheel
		EnhancedInput->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &ATopDownPawn::ZoomCamera);
	}
}

void ATopDownPawn::OnLeftMousePressed()
{
	// UE_LOG(LogTemp, Log, TEXT("TopDownPawn: Left Mouse Pressed"));
	if (!BuildManagerRef)
	{
		UE_LOG(LogTemp, Log, TEXT("TopDownPawn: BuildManagerRef is null!"));
		return;
	}

	if (CurrentMode == EGameMode::Build)
	{
		UE_LOG(LogTemp, Log, TEXT("TopDownPawn: Attempting TryPlaceTower"));
		BuildManagerRef->TryPlaceTower();
	}
	else if (CurrentMode == EGameMode::Delete)
	{
		UE_LOG(LogTemp, Log, TEXT("TopDownPawn: Attempting TryDeleteTower"));
		BuildManagerRef->TryDeleteTower();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TopDownPawn: No mode active"));
	}
}

void ATopDownPawn::OnRightMousePressed()
{
	if (!bCameraInterpolationActive) // Cant rotate when lerping
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
		// Reset mouse to center?
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
	if (FMath::IsNearlyZero(Input.X) && FMath::IsNearlyZero(Input.Y)) return;
	
	// Get the SpringArm rotation to orient movement with the camera
	FRotator CameraRotation = SpringArm->GetComponentRotation();
	FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f); // Only yaw affects movement
	FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	FVector MoveDirection = (Forward * Input.Y + Right * Input.X).GetSafeNormal();
	AddMovementInput(MoveDirection, 1.0f);
}

void ATopDownPawn::RotateCamera(const FInputActionValue& Value)
{
	if (!bCameraRotationActive || !SpringArm)
		return;

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
		bCameraRotationActive = false;

	if (!bCameraInterpolationActive)
	{
		UE_LOG(LogTemp, Log, TEXT("Toggle View: %s"), bGodViewEnabled ? TEXT("Normal View") : TEXT("God View"));
		bGodViewEnabled = !bGodViewEnabled;
		TargetPitch = bGodViewEnabled ? -90.f : BaseCameraAngle;
		bCameraInterpolationActive = true; // Activates on tick and needs to be last
	}
}

void ATopDownPawn::ToggleMode(EGameMode NewMode)
{
	if (CurrentMode == NewMode)
		CurrentMode = EGameMode::None; // Toggle off
	else
		CurrentMode = NewMode;

	if (BuildManagerRef)
		BuildManagerRef->SetMode(CurrentMode);

	if (GridManagerRef && BuildManagerRef)
	{
		for (int32 X = 0; X < GridManagerRef->GetGridWidth(); ++X)
		{
			for (int32 Y = 0; Y < GridManagerRef->GetGridHeight(); ++Y)
			{
				FTileData Tile;

				if (GridManagerRef->GetTileSafe(X, Y, Tile))
				{
					ETileVisualState NewVisualState = BuildManagerRef->GetVisualStateForTile(Tile, false);
					GridManagerRef->SetTileVisual(X, Y, NewVisualState);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TopDownView: Mode set to %s"),
		*UEnum::GetValueAsString(CurrentMode));
}

void ATopDownPawn::ToggleBuildMode()
{
	ToggleMode(EGameMode::Build);
	UE_LOG(LogTemp, Log, TEXT("TopDownPawn: ToggleBuildMode called"))
}

void ATopDownPawn::ToggleDeleteMode()
{
	ToggleMode(EGameMode::Delete);
	UE_LOG(LogTemp, Log, TEXT("TopDownPawn: ToggleDeleteMode called"))
}

void ATopDownPawn::ZoomCamera(const FInputActionValue& Value)
{
	if (!SpringArm)
		return;

	float InputValue = Value.Get<float>();
	if (bInvertedScrollDirection)
		InputValue = InputValue * -1;

	ZoomLevel += InputValue * ZoomSpeed;
	ZoomLevel = FMath::Clamp(ZoomLevel, 0, 1500);
	SpringArm->TargetArmLength = ZoomLevel;
}
