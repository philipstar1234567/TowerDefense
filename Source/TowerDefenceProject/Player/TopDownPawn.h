	
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HUD/EndGameWidget.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Player/PlayerResourceState.h"
#include "Grid/GridManager.h"
#include "Grid/BuildManager.h"
#include "Blueprint/UserWidget.h"
#include "TopDownPawn.generated.h"


// Forward declaration
class UPauseMenuWidget;

/**
 * @brief Player-controlled top-down pawn used for camera movement,
 * building actions, UI toggling, and general game interaction.
 */
UCLASS()
class TOWERDEFENCEPROJECT_API ATopDownPawn : public APawn
{
	GENERATED_BODY()

public:
	/** @brief Constructor. Initializes components and default values. */
	ATopDownPawn();

	/** @brief Called at game start. */
	virtual void BeginPlay() override;

	/**
	 * @brief Binds input actions from the Enhanced Input system.
	 * @param PlayerInputComponent Input component provided by the engine.
	 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Pointer to scenecomponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	/** Pointer to SprintArmComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm;

	/** Pointer to CameraComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	/** Pointer to FloatingPawnMovement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFloatingPawnMovement* FloatingPawnMovement;

	/** Stores a Subclass of GridManager (The BP child) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TSubclassOf<AGridManager> GridManagerClass;

	/** Pointer to GridManager */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	AGridManager* GridManagerRef;

	/** Stores a Subclass of BuildManager (The BP child) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TSubclassOf<ABuildManager> BuildManagerClass;
	
	/** Pointer to BuildManager */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	ABuildManager* BuildManagerRef;

	/** Stores a Subclass of EndGameWidget */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UEndGameWidget> EndGameWidgetClass;

	/** The speed of the camera zoom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed;

	/** The speed of the camera rotation on Yaw */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float YawRotationSpeed;

	/** The starting angle of the camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BaseCameraAngle;

	/** If the scroll wheel should be inverted for zoom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bInvertedScrollDirection;

	/** If the scroll wheel should be inverted for zoom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UPauseMenuWidget> PauseMenuClass;

	/** Pointer to the PauseMenuWidget */
	UPROPERTY()
	UPauseMenuWidget* PauseMenuInstance;

	/** @brief Toggles build mode on/off. */
	UFUNCTION()
	void ToggleBuildMode();

	/** @brief Toggles delete mode on/off. */
	UFUNCTION()
	void ToggleDeleteMode();

	/** @brief Toggled the Pause mode */
	UFUNCTION()
	void TogglePause();

	/** @brief Handles end-game defeat logic and UI. */
	UFUNCTION()
	void HandleDefeat();
	
	/** @brief Current gameplay mode (None, Build, Delete) */
	UPROPERTY(BlueprintReadOnly, Category = "Mode")
	EGameMode CurrentMode = EGameMode::None;

protected:
	/** @brief Per-frame update. */
	virtual void Tick(float DeltaTime) override;

	/** Internal state variables */
	float ZoomLevel;
	float TargetPitch;
	bool bCameraRotationActive;
	bool bCameraInterpolationActive;
	bool bGodViewEnabled;
	bool bPauseMenuToggle;
	FVector2D ViewportCenter;

	/** Input Action Pointers */
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

	UPROPERTY(EditDefaultsOnly, Category = "Input|Keyboard")
	class UInputAction* IA_ToggleDeleteMode;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Keyboard")
	class UInputAction* IA_TogglePauseMode;

	UPROPERTY(EditAnywhere, Category = "Input|Keyboard")
	class UInputAction* IA_ToggleView;

	UPROPERTY(EditAnywhere, Category = "Input|Mouse")
	class UInputAction* IA_Zoom;


private:
	/** @brief Called when left mouse button is pressed. */
	UFUNCTION()
	void OnLeftMousePressed();

	/** @brief Called when right mouse button is pressed. */
	UFUNCTION()
	void OnRightMousePressed();

	/** @brief Called when right mouse button is released. */
	UFUNCTION()
	void OnRightMouseReleased();

	/**
	* @brief Moves the pawn based on WASD input.
	* @param Value Move direction and magnitude.
	*/
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	/**
	* @brief Rotates the camera horizontally.
	* @param Value Mouse X-axis delta.
	*/
	UFUNCTION()
	void RotateCamera(const FInputActionValue& Value);

	/** @brief Toggles God View / alternate camera angle. */
	UFUNCTION()
	void ToggleView();

	/**
	* @brief Switches to a new game mode.
	* @param NewMode Mode to switch into.
	*/
	UFUNCTION()
	void ToggleMode(EGameMode NewMode);

	/**
	* @brief Zooms the camera in/out.
	* @param Value Scroll wheel input.
	*/
	UFUNCTION()
	void ZoomCamera(const FInputActionValue& Value);
};
