// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "FTPShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AFTPShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Optional first person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

	/** Optional first person mesh, intended for arms-only meshes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Optional enhanced input action for perspective switching */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* TogglePerspectiveAction;

	/** Optional enhanced input action for aiming */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ToggleAimAction;

	/** Start the character in first person instead of third person */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bStartInFirstPersonPerspective = false;

	/** Key used to toggle between first and third person when no input asset is assigned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FKey TogglePerspectiveKey;

	/** Key used to toggle aiming when no input asset is assigned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FKey ToggleAimKey;

	/** Third person camera distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta = (ClampMin = 0, Units = "cm"))
	float ThirdPersonCameraDistance = 300.0f;

	/** Third person camera boom offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FVector ThirdPersonCameraBoomOffset = FVector(0.0f, 50.0f, 68.0f);

	/** Third person field of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta = (ClampMin = 1.0))
	float ThirdPersonFOV = 80.0f;

	/** First person camera offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FVector FirstPersonCameraOffset = FVector(0.0f, 0.0f, 64.0f);

	/** First person field of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta = (ClampMin = 1.0))
	float FirstPersonFOV = 90.0f;

	/** Relative location for an optional first person mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FVector FirstPersonMeshOffset = FVector(-30.0f, 0.0f, -150.0f);

	/** When enabled, the first person mesh auto-copies the third person skeletal mesh if none is assigned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bAutoCopyThirdPersonMeshToFirstPersonMesh = false;

	/** When enabled, the first person mesh auto-copies the third person anim class if none is assigned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bAutoCopyThirdPersonAnimClassToFirstPersonMesh = false;

	/** Whether the local character is currently using the first person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	bool bIsFirstPersonPerspective = false;

	/** Third person aim camera distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Aim", meta = (ClampMin = 0, Units = "cm"))
	float AimThirdPersonCameraDistance = 180.0f;

	/** Third person aim camera boom offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Aim")
	FVector AimThirdPersonCameraBoomOffset = FVector(0.0f, 65.0f, 68.0f);

	/** Third person aim field of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Aim", meta = (ClampMin = 1.0))
	float AimThirdPersonFOV = 65.0f;

	/** First person aim field of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Aim", meta = (ClampMin = 1.0))
	float AimFirstPersonFOV = 75.0f;

	/** Camera interpolation speed when switching aim states */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Aim", meta = (ClampMin = 0.1))
	float CameraInterpolationSpeed = 12.0f;

	/** Whether the local character is currently aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera|Aim")
	bool bIsAiming = false;

public:

	/** Constructor */
	AFTPShooterCharacter();	

protected:

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Toggle between first and third person */
	void TogglePerspective();

	/** Apply a specific perspective state */
	void SetPerspective(bool bEnableFirstPerson);

	/** Toggle aim state */
	void ToggleAim();

	/** Enter aim state */
	void StartAim();

	/** Exit aim state */
	void StopAim();

	/** Apply aiming state */
	void SetAim(bool bEnableAim);

	/** Update active camera transform/FOV toward the current target state */
	void UpdateCameraState(float DeltaSeconds, bool bInstant = false);

	/** Keep the first person mesh synced to the main mesh for editor preview and runtime */
	void RefreshFirstPersonMesh();

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="Camera")
	bool IsFirstPersonPerspective() const { return bIsFirstPersonPerspective; }

	UFUNCTION(BlueprintCallable, Category="Camera|Aim")
	bool IsAiming() const { return bIsAiming; }

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns FirstPersonCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	/** Returns FirstPersonMesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
};
