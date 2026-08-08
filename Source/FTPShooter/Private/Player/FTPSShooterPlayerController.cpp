// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/FTPSShooterPlayerController.h"

#include "Character/FTPShooterCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

AFTPSShooterPlayerController::AFTPSShooterPlayerController()
{
	bPawnAlive = true;
}

void AFTPSShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AFTPSShooterPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	OnPlayerStateReplicated.Broadcast();
}

void AFTPSShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	bPawnAlive = IsValid(InPawn);
}

void AFTPSShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (ShooterIMC)
		{
			Subsystem->AddMappingContext(ShooterIMC, 0);
		}
	}

	if (UEnhancedInputComponent* ShooterInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{
			ShooterInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
		}
		if (LookAction)
		{
			ShooterInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
		}
		if (CrouchAction)
		{
			ShooterInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::Input_Crouch);
		}
		if (JumpAction)
		{
			ShooterInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Input_Jump);
		}
		if (TogglePerspectiveAction)
		{
			ShooterInputComponent->BindAction(TogglePerspectiveAction, ETriggerEvent::Started, this, &ThisClass::Input_TogglePerspective);
		}
		if (AimAction)
		{
			ShooterInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::Input_AimStarted);
			ShooterInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::Input_AimEnded);
			ShooterInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &ThisClass::Input_AimEnded);
		}
	}
}

void AFTPSShooterPlayerController::Input_Crouch()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
		{
			MovementComponent->bWantsToCrouch = !MovementComponent->bWantsToCrouch;
		}
	}
}

void AFTPSShooterPlayerController::Input_Jump()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->Jump();
	}
}

void AFTPSShooterPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
		ShooterCharacter->DoMove(InputAxisVector.X, InputAxisVector.Y);
	}
}

void AFTPSShooterPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
		ShooterCharacter->DoLook(InputAxisVector.X, InputAxisVector.Y);
	}
}

void AFTPSShooterPlayerController::Input_TogglePerspective()
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		ShooterCharacter->TogglePerspective();
	}
}

void AFTPSShooterPlayerController::Input_AimStarted()
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		ShooterCharacter->StartAim();
	}
}

void AFTPSShooterPlayerController::Input_AimEnded()
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		ShooterCharacter->StopAim();
	}
}
