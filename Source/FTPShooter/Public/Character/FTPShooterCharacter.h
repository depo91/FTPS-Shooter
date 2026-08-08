// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/FTPSPlayerInterface.h"
#include "Logging/LogMacros.h"
#include "FTPShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UFTPSCombatComponent;
class UInputAction;
class AFTPSWeapon;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponFirstReplicated, AFTPSWeapon*, Weapon, bool, bTargetingPlayer);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AFTPShooterCharacter : public ACharacter, public IFTPSPlayerInterface
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

	/** First person mesh, intended for arms-only meshes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** Third person mesh alias (Character inherited mesh) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh3P;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Combat")
	TObjectPtr<UFTPSCombatComponent> Combat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Aiming")
	float DefaultFieldOfView;

	UFUNCTION(BlueprintImplementableEvent)
	void OnAim(bool bAiming);

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

	/** Bone hidden only in FPS so full body stays visible without head clipping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FName FirstPersonHiddenBoneName = TEXT("head");

	/** Optional upper-body root bone to hide in FPS (keeps legs visible, removes torso clipping). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FName FirstPersonHiddenUpperBodyBoneName = TEXT("spine_01");

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

	/** PlayerInterface */
	virtual FName GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponType) const override;
	virtual USkeletalMeshComponent* GetMesh1P_Implementation() const override;
	virtual USkeletalMeshComponent* GetMesh3P_Implementation() const override;
	virtual void WeaponReplicated_Implementation() override;
	virtual AFTPSWeapon* GetCurrentWeapon_Implementation() override;
	virtual int32 GetReserveAmmo_Implementation() const override;
	virtual void Notify_CycleWeapon_Implementation() override;
	virtual void Notify_ReloadWeapon_Implementation() override;
	virtual void AddAmmo_Implementation(const FGameplayTag& WeaponType, int32 AmmoAmount) override;
	virtual bool DoDamage_Implementation(float DamageAmount, AActor* DamageInstigator) override;
	/** ~PlayerInterface */

protected:

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;

	/** Initialize key fallback bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Update active camera transform/FOV toward the current target state */
	void UpdateCameraState(float DeltaSeconds, bool bInstant = false);

	/** Keep the first person mesh synced to the main mesh for editor preview and runtime */
	void RefreshFirstPersonMesh();

private:

	void Input_CycleWeapon();
	void Input_ReloadWeapon();
	void Input_FireWeapon_Pressed();
	void Input_FireWeapon_Released();
	void Input_Aim_Pressed();
	void Input_Aim_Released();

	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> CycleWeaponAction;

	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> FireWeaponAction;

	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> ReloadWeaponAction;

	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> AimWeaponAction;

public:
	UPROPERTY(BlueprintAssignable)
	FWeaponFirstReplicated OnWeaponFirstReplicated;

	UFUNCTION(BlueprintCallable, Category="FPS|Combat")
	bool HasWeaponFirstReplicated() const { return bWeaponFirstReplicated; }

private:
	bool bWeaponFirstReplicated = false;

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

	/** Toggle between first and third person */
	UFUNCTION(BlueprintCallable, Category="Input")
	void TogglePerspective();

	/** Enter aim state */
	UFUNCTION(BlueprintCallable, Category="Input")
	void StartAim();

	/** Exit aim state */
	UFUNCTION(BlueprintCallable, Category="Input")
	void StopAim();

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
	FORCEINLINE class USkeletalMeshComponent* GetFirstPersonMesh() const { return Mesh1P; }

protected:

	/** Apply a specific perspective state */
	void SetPerspective(bool bEnableFirstPerson);

	/** Toggle aim state */
	void ToggleAim();

	/** Apply aiming state */
	void SetAim(bool bEnableAim);
};
