// Copyright Epic Games, Inc. All Rights Reserved.

#include "FTPShooterCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "FTPShooter.h"

AFTPShooterCharacter::AFTPShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetMesh()->bCastHiddenShadow = true;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = ThirdPersonCameraDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeLocation(ThirdPersonCameraBoomOffset);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = ThirdPersonFOV;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
	FirstPersonCamera->FieldOfView = FirstPersonFOV;

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonMesh->SetCastShadow(false);
	FirstPersonMesh->SetRelativeLocation(FirstPersonMeshOffset);
	FirstPersonMesh->SetVisibility(true, true);

	TogglePerspectiveKey = EKeys::V;
	ToggleAimKey = EKeys::RightMouseButton;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AFTPShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	CameraBoom->TargetArmLength = ThirdPersonCameraDistance;
	CameraBoom->SetRelativeLocation(ThirdPersonCameraBoomOffset);
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
	RefreshFirstPersonMesh();

	SetPerspective(bStartInFirstPersonPerspective);
	UpdateCameraState(0.0f, true);
}

void AFTPShooterCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshFirstPersonMesh();
}

void AFTPShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCameraState(DeltaSeconds);
}

void AFTPShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFTPShooterCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AFTPShooterCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFTPShooterCharacter::Look);

		if (TogglePerspectiveAction)
		{
			EnhancedInputComponent->BindAction(TogglePerspectiveAction, ETriggerEvent::Started, this, &AFTPShooterCharacter::TogglePerspective);
		}

		if (ToggleAimAction)
		{
			EnhancedInputComponent->BindAction(ToggleAimAction, ETriggerEvent::Started, this, &AFTPShooterCharacter::StartAim);
			EnhancedInputComponent->BindAction(ToggleAimAction, ETriggerEvent::Completed, this, &AFTPShooterCharacter::StopAim);
			EnhancedInputComponent->BindAction(ToggleAimAction, ETriggerEvent::Canceled, this, &AFTPShooterCharacter::StopAim);
		}
	}
	else
	{
		UE_LOG(LogFTPShooter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	PlayerInputComponent->BindKey(TogglePerspectiveKey, IE_Pressed, this, &AFTPShooterCharacter::TogglePerspective);
	PlayerInputComponent->BindKey(ToggleAimKey, IE_Pressed, this, &AFTPShooterCharacter::StartAim);
	PlayerInputComponent->BindKey(ToggleAimKey, IE_Released, this, &AFTPShooterCharacter::StopAim);
}

void AFTPShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AFTPShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AFTPShooterCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AFTPShooterCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AFTPShooterCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AFTPShooterCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AFTPShooterCharacter::TogglePerspective()
{
	SetPerspective(!bIsFirstPersonPerspective);
}

void AFTPShooterCharacter::SetPerspective(bool bEnableFirstPerson)
{
	bIsFirstPersonPerspective = bEnableFirstPerson;

	if (bIsFirstPersonPerspective)
	{
		if (Controller)
		{
			FRotator ControlRotation = Controller->GetControlRotation();
			ControlRotation.Yaw = GetActorRotation().Yaw;
			Controller->SetControlRotation(ControlRotation);
		}

		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

		if (FollowCamera)
		{
			FollowCamera->Deactivate();
		}

		if (FirstPersonCamera)
		{
			FirstPersonCamera->Activate();
		}

		GetMesh()->SetVisibility(false, true);

		if (FirstPersonMesh)
		{
			FirstPersonMesh->SetVisibility(FirstPersonMesh->GetSkeletalMeshAsset() != nullptr, true);
		}
	}
	else
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;

		if (FirstPersonCamera)
		{
			FirstPersonCamera->Deactivate();
		}

		if (FollowCamera)
		{
			FollowCamera->Activate();
		}

		GetMesh()->SetVisibility(true, true);

		if (FirstPersonMesh)
		{
			FirstPersonMesh->SetVisibility(false, true);
		}
	}

	UpdateCameraState(0.0f, true);
}

void AFTPShooterCharacter::ToggleAim()
{
	SetAim(!bIsAiming);
}

void AFTPShooterCharacter::StartAim()
{
	SetAim(true);
}

void AFTPShooterCharacter::StopAim()
{
	SetAim(false);
}

void AFTPShooterCharacter::SetAim(bool bEnableAim)
{
	bIsAiming = bEnableAim;
	UpdateCameraState(0.0f, false);
}

void AFTPShooterCharacter::UpdateCameraState(float DeltaSeconds, bool bInstant)
{
	if (!CameraBoom || !FollowCamera || !FirstPersonCamera)
	{
		return;
	}

	const float InterpSpeed = bInstant ? 0.0f : CameraInterpolationSpeed;
	const bool bInterpolate = !bInstant && InterpSpeed > 0.0f;

	const float TargetArmLength = bIsAiming ? AimThirdPersonCameraDistance : ThirdPersonCameraDistance;
	const FVector TargetBoomOffset = bIsAiming ? AimThirdPersonCameraBoomOffset : ThirdPersonCameraBoomOffset;
	const float TargetThirdPersonFOV = bIsAiming ? AimThirdPersonFOV : ThirdPersonFOV;
	const float TargetFirstPersonFOV = bIsAiming ? AimFirstPersonFOV : FirstPersonFOV;

	if (bInterpolate)
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaSeconds, InterpSpeed);
		CameraBoom->SetRelativeLocation(FMath::VInterpTo(CameraBoom->GetRelativeLocation(), TargetBoomOffset, DeltaSeconds, InterpSpeed));
		FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetThirdPersonFOV, DeltaSeconds, InterpSpeed));
		FirstPersonCamera->SetFieldOfView(FMath::FInterpTo(FirstPersonCamera->FieldOfView, TargetFirstPersonFOV, DeltaSeconds, InterpSpeed));
	}
	else
	{
		CameraBoom->TargetArmLength = TargetArmLength;
		CameraBoom->SetRelativeLocation(TargetBoomOffset);
		FollowCamera->SetFieldOfView(TargetThirdPersonFOV);
		FirstPersonCamera->SetFieldOfView(TargetFirstPersonFOV);
	}
}

void AFTPShooterCharacter::RefreshFirstPersonMesh()
{
	if (!FirstPersonMesh || !GetMesh())
	{
		return;
	}

	if (bAutoCopyThirdPersonMeshToFirstPersonMesh
		&& FirstPersonMesh->GetSkeletalMeshAsset() == nullptr
		&& GetMesh()->GetSkeletalMeshAsset() != nullptr)
	{
		FirstPersonMesh->SetSkeletalMesh(GetMesh()->GetSkeletalMeshAsset());
	}

	if (bAutoCopyThirdPersonAnimClassToFirstPersonMesh
		&& FirstPersonMesh->GetAnimClass() == nullptr
		&& GetMesh()->GetAnimClass() != nullptr)
	{
		FirstPersonMesh->SetAnimInstanceClass(GetMesh()->GetAnimClass());
	}

	if (UWorld* World = GetWorld())
	{
		const bool bIsEditorPreviewWorld =
			World->WorldType == EWorldType::Editor ||
			World->WorldType == EWorldType::EditorPreview ||
			World->WorldType == EWorldType::Inactive;

		if (bIsEditorPreviewWorld)
		{
			FirstPersonMesh->SetVisibility(FirstPersonMesh->GetSkeletalMeshAsset() != nullptr, true);
		}
	}
}
