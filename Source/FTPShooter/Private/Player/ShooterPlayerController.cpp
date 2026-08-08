// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/ShooterPlayerController.h"

#include "Character/FTPShooterCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

AShooterPlayerController::AShooterPlayerController()
{
	bPawnAlive = true;
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	OnPlayerStateReplicated.Broadcast();
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	bPawnAlive = IsValid(InPawn);
}

void AShooterPlayerController::SetupInputComponent()
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
	}
}

void AShooterPlayerController::Input_Crouch()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
		{
			MovementComponent->bWantsToCrouch = !MovementComponent->bWantsToCrouch;
		}
	}
}

void AShooterPlayerController::Input_Jump()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->Jump();
	}
}

void AShooterPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
		ShooterCharacter->DoMove(InputAxisVector.X, InputAxisVector.Y);
	}
}

void AShooterPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	if (AFTPShooterCharacter* ShooterCharacter = Cast<AFTPShooterCharacter>(GetCharacter()))
	{
		const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
		ShooterCharacter->DoLook(InputAxisVector.X, InputAxisVector.Y);
	}
}
