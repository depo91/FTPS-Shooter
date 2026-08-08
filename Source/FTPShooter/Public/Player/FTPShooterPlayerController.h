// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FTPShooterPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
struct FInputActionValue;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AFTPShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

private:

	/** Movement input action */
	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	TObjectPtr<UInputAction> MoveAction;

	/** Look input action */
	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	TObjectPtr<UInputAction> LookAction;

	/** Jump input action */
	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	TObjectPtr<UInputAction> JumpAction;

	/** Crouch input action */
	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	TObjectPtr<UInputAction> CrouchAction;

	/** Perspective toggle input action */
	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	TObjectPtr<UInputAction> TogglePerspectiveAction;

	/** Aim input action (hold) */
	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	TObjectPtr<UInputAction> AimAction;

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_Jump();
	void Input_Crouch();
	void Input_TogglePerspective();
	void Input_AimStarted();
	void Input_AimEnded();
};
