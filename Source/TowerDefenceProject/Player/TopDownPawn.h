
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Player/PlayerResourceState.h"
#include "Grid/GridManager.h"
#include "Grid/BuildManager.h"
#include "TopDownPawn.generated.h"

UCLASS()
class TOWERDEFENCEPROJECT_API ATopDownPawn : public APawn
{
	GENERATED_BODY()

public:
	ATopDownPawn();
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFloatingPawnMovement* FloatingPawnMovement;

	// GridManager
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TSubclassOf<AGridManager> GridManagerClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	AGridManager* GridManagerRef;

	// BuildManager
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TSubclassOf<ABuildManager> BuildManagerClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	ABuildManager* BuildManagerRef;

	// Public Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float YawRotationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BaseCameraAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bInvertedScrollDirection;

protected:
	virtual void Tick(float DeltaTime) override;

	// Private Variables
	float ZoomLevel;
	float TargetPitch;
	bool bCameraRotationActive;
	bool bCameraInterpolationActive;
	bool bGodViewEnabled;
	FVector2D ViewportCenter;

	UPROPERTY(BlueprintReadOnly, Category = "Mode")
	EGameMode CurrentMode = EGameMode::None;

	// Input Mapping Context and Input Actions
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Mouse")
	class UInputAction* IA_LeftMouseB;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Mouse")
	class UInputAction* IA_RightMouseB;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Mouse")
	class UInputAction* IA_MouseX;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Keyboard")
	class UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Keyboard")
	class UInputAction* IA_ToggleBuildMode;

	UPROPERTY(EditDefaultsOnly, Category = "Inputs|Keyboard")
	class UInputAction* IA_ToggleDeleteMode;

	UPROPERTY(EditAnywhere, Category = "Input|Keyboard")
	class UInputAction* IA_ToggleView;

	UPROPERTY(EditAnywhere, Category = "Input|Mouse")
	class UInputAction* IA_Zoom;

private:
	UFUNCTION()
	void OnLeftMousePressed();

	UFUNCTION()
	void OnRightMousePressed();

	UFUNCTION()
	void OnRightMouseReleased();

	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void RotateCamera(const FInputActionValue& Value);

	UFUNCTION()
	void ToggleView();

	UFUNCTION()
	void ToggleMode(EGameMode NewMode);

	UFUNCTION()
	void ToggleBuildMode();

	UFUNCTION()
	void ToggleDeleteMode();

	UFUNCTION()
	void ZoomCamera(const FInputActionValue& Value);
};
