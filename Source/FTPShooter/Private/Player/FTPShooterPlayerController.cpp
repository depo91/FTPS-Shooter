// Copyright Epic Games, Inc. All Rights Reserved.


#include "Player/FTPShooterPlayerController.h"
#include "Character/FTPShooterCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "FTPShooter.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AFTPShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogFTPShooter, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AFTPShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	if (UEnhancedInputComponent* ShooterInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{
			ShooterInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFTPShooterPlayerController::Input_Move);
		}

		if (LookAction)
		{
			ShooterInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFTPShooterPlayerController::Input_Look);
		}

		if (JumpAction)
		{
			ShooterInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFTPShooterPlayerController::Input_Jump);
		}

		if (CrouchAction)
		{
			ShooterInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AFTPShooterPlayerController::Input_Crouch);
		}

		if (TogglePerspectiveAction)
		{
			ShooterInputComponent->BindAction(TogglePerspectiveAction, ETriggerEvent::Started, this, &AFTPShooterPlayerController::Input_TogglePerspective);
		}

		if (AimAction)
		{
			ShooterInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AFTPShooterPlayerController::Input_AimStarted);
			ShooterInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AFTPShooterPlayerController::Input_AimEnded);
			ShooterInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &AFTPShooterPlayerController::Input_AimEnded);
		}
	}
	else
	{
		UE_LOG(LogFTPShooter, Error, TEXT("'%s' failed to find an Enhanced Input component."), *GetNameSafe(this));
	}
}

bool AFTPShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AFTPShooterPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
		ShooterCharacter->DoMove(InputAxisVector.X, InputAxisVector.Y);
	}
}

void AFTPShooterPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
		ShooterCharacter->DoLook(InputAxisVector.X, InputAxisVector.Y);
	}
}

void AFTPShooterPlayerController::Input_Jump()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
		{
			if (MovementComponent->bWantsToCrouch)
			{
				MovementComponent->bWantsToCrouch = false;
				return;
			}
		}

		ControlledCharacter->Jump();
	}
}

void AFTPShooterPlayerController::Input_Crouch()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
		{
			MovementComponent->bWantsToCrouch = !MovementComponent->bWantsToCrouch;
		}
	}
}

void AFTPShooterPlayerController::Input_TogglePerspective()
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		ShooterCharacter->TogglePerspective();
	}
}

void AFTPShooterPlayerController::Input_AimStarted()
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		ShooterCharacter->StartAim();
	}
}

void AFTPShooterPlayerController::Input_AimEnded()
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		ShooterCharacter->StopAim();
	}
}
