// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/FTPShooterCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Combat/FTPSCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/FTPSWeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputCoreTypes.h"
#include "FTPShooter.h"
#include "Weapon/FTPSWeapon.h"

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
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
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

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetupAttachment(FirstPersonCamera);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetOwnerNoSee(false);
	Mesh1P->SetCastShadow(false);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Mesh1P->SetRelativeLocation(FirstPersonMeshOffset);
	Mesh1P->SetVisibility(true, true);

	Mesh3P = GetMesh();
	Mesh3P->SetOnlyOwnerSee(false);
	Mesh3P->SetOwnerNoSee(false);
	Mesh3P->bReceivesDecals = false;

	Combat = CreateDefaultSubobject<UFTPSCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	TogglePerspectiveKey = EKeys::V;
	ToggleAimKey = EKeys::RightMouseButton;
	DefaultFieldOfView = 90.0f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AFTPShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	FirstPersonCamera->SetFieldOfView(DefaultFieldOfView);
	CameraBoom->TargetArmLength = ThirdPersonCameraDistance;
	CameraBoom->SetRelativeLocation(ThirdPersonCameraBoomOffset);
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
	RefreshFirstPersonMesh();

	SetPerspective(bStartInFirstPersonPerspective);
	UpdateCameraState(0.0f, true);
}

void AFTPShooterCharacter::BeginDestroy()
{
	Super::BeginDestroy();

	if (IsValid(Combat))
	{
		Combat->DestroyInventory();
	}
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

void AFTPShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsValid(Combat))
	{
		Combat->SpawnInventory();

		if (AFTPSWeapon* Weapon = Combat->GetCurrentWeapon())
		{
			bWeaponFirstReplicated = true;
			OnWeaponFirstReplicated.Broadcast(Weapon, Combat->bHitPlayer);
		}
	}
}

void AFTPShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* ShooterInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	if (CycleWeaponAction)
	{
		ShooterInputComponent->BindAction(CycleWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_CycleWeapon);
	}

	if (FireWeaponAction)
	{
		ShooterInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_FireWeapon_Pressed);
		ShooterInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &ThisClass::Input_FireWeapon_Released);
	}

	if (AimWeaponAction)
	{
		ShooterInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_Aim_Pressed);
		ShooterInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Completed, this, &ThisClass::Input_Aim_Released);
	}

	if (ReloadWeaponAction)
	{
		ShooterInputComponent->BindAction(ReloadWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_ReloadWeapon);
	}

	PlayerInputComponent->BindKey(TogglePerspectiveKey, IE_Pressed, this, &AFTPShooterCharacter::TogglePerspective);
	PlayerInputComponent->BindKey(ToggleAimKey, IE_Pressed, this, &AFTPShooterCharacter::StartAim);
	PlayerInputComponent->BindKey(ToggleAimKey, IE_Released, this, &AFTPShooterCharacter::StopAim);
}

FName AFTPShooterCharacter::GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponType) const
{
	checkf(Combat && Combat->WeaponData, TEXT("No Weapon Data Asset - Please fill out BP_ShooterCharacter"));

	if (const FName* AttachPoint = Combat->WeaponData->GripPoints.Find(WeaponType))
	{
		return *AttachPoint;
	}

	UE_LOG(LogTemp, Warning, TEXT("GetWeaponAttachPoint: No GripPoint entry found for WeaponType '%s' in DA_WeaponData. Falling back to 'hand_r'. Set WeaponType on the weapon Blueprint and/or add a GripPoints entry to fix this."), *WeaponType.ToString());
	return TEXT("hand_r");
}

USkeletalMeshComponent* AFTPShooterCharacter::GetMesh1P_Implementation() const
{
	return Mesh1P;
}

USkeletalMeshComponent* AFTPShooterCharacter::GetMesh3P_Implementation() const
{
	return Mesh3P;
}

void AFTPShooterCharacter::WeaponReplicated_Implementation()
{
	if (Combat && Combat->GetCurrentWeapon())
	{
		bWeaponFirstReplicated = true;
		OnWeaponFirstReplicated.Broadcast(Combat->GetCurrentWeapon(), Combat->bHitPlayer);
	}
}

AFTPSWeapon* AFTPShooterCharacter::GetCurrentWeapon_Implementation()
{
	return Combat ? Combat->GetCurrentWeapon() : nullptr;
}

int32 AFTPShooterCharacter::GetReserveAmmo_Implementation() const
{
	return Combat ? Combat->GetReserveAmmo() : 0;
}

void AFTPShooterCharacter::Notify_CycleWeapon_Implementation()
{
	if (Combat)
	{
		Combat->Initiate_CycleWeapon();
	}
}

void AFTPShooterCharacter::Notify_ReloadWeapon_Implementation()
{
	if (Combat)
	{
		Combat->Initiate_ReloadWeapon();
	}
}

void AFTPShooterCharacter::AddAmmo_Implementation(const FGameplayTag& WeaponType, int32 AmmoAmount)
{
	if (Combat && Combat->WeaponData && Combat->WeaponData->GripPoints.Contains(WeaponType))
	{
		Combat->AddReserveAmmo(AmmoAmount);
	}
}

bool AFTPShooterCharacter::DoDamage_Implementation(float DamageAmount, AActor* DamageInstigator)
{
	return DamageAmount > 0.f && IsValid(DamageInstigator);
}

void AFTPShooterCharacter::Input_CycleWeapon()
{
	Combat->Initiate_CycleWeapon();
}

void AFTPShooterCharacter::Input_ReloadWeapon()
{
	Combat->Initiate_ReloadWeapon();
}

void AFTPShooterCharacter::Input_FireWeapon_Pressed()
{
	Combat->Initiate_FireWeapon_Pressed();
}

void AFTPShooterCharacter::Input_FireWeapon_Released()
{
	Combat->Initiate_FireWeapon_Released();
}

void AFTPShooterCharacter::Input_Aim_Pressed()
{
	Combat->Initiate_Aim_Pressed();
	if (bIsFirstPersonPerspective)
	{
		OnAim(true);
	}
}

void AFTPShooterCharacter::Input_Aim_Released()
{
	Combat->Initiate_Aim_Released();
	if (bIsFirstPersonPerspective)
	{
		OnAim(false);
	}
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

		Mesh3P->SetOnlyOwnerSee(false);
		Mesh3P->SetOwnerNoSee(false);
		Mesh3P->SetVisibility(true, true);
		if (!FirstPersonHiddenBoneName.IsNone())
		{
			Mesh3P->HideBoneByName(FirstPersonHiddenBoneName, EPhysBodyOp::PBO_None);
		}
		if (!FirstPersonHiddenUpperBodyBoneName.IsNone())
		{
			Mesh3P->HideBoneByName(FirstPersonHiddenUpperBodyBoneName, EPhysBodyOp::PBO_None);
		}

		if (Mesh1P)
		{
			Mesh1P->SetVisibility(Mesh1P->GetSkeletalMeshAsset() != nullptr, true);
		}

		OnAim(bIsAiming);
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

		Mesh3P->SetOnlyOwnerSee(false);
		Mesh3P->SetOwnerNoSee(false);
		Mesh3P->SetVisibility(true, true);
		if (!FirstPersonHiddenBoneName.IsNone())
		{
			Mesh3P->UnHideBoneByName(FirstPersonHiddenBoneName);
		}
		if (!FirstPersonHiddenUpperBodyBoneName.IsNone())
		{
			Mesh3P->UnHideBoneByName(FirstPersonHiddenUpperBodyBoneName);
		}

		if (Mesh1P)
		{
			Mesh1P->SetVisibility(false, true);
		}

		OnAim(false);
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
	if (!Mesh1P || !GetMesh())
	{
		return;
	}

	if (bAutoCopyThirdPersonMeshToFirstPersonMesh
		&& Mesh1P->GetSkeletalMeshAsset() == nullptr
		&& GetMesh()->GetSkeletalMeshAsset() != nullptr)
	{
		Mesh1P->SetSkeletalMesh(GetMesh()->GetSkeletalMeshAsset());
	}

	if (bAutoCopyThirdPersonAnimClassToFirstPersonMesh
		&& Mesh1P->GetAnimClass() == nullptr
		&& GetMesh()->GetAnimClass() != nullptr)
	{
		Mesh1P->SetAnimInstanceClass(GetMesh()->GetAnimClass());
	}

	if (UWorld* World = GetWorld())
	{
		const bool bIsEditorPreviewWorld =
			World->WorldType == EWorldType::Editor ||
			World->WorldType == EWorldType::EditorPreview ||
			World->WorldType == EWorldType::Inactive;

		if (bIsEditorPreviewWorld)
		{
			Mesh1P->SetVisibility(Mesh1P->GetSkeletalMeshAsset() != nullptr, true);
		}
	}
}
