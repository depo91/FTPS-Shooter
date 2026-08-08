// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterTypes/FTPSShooterTypes.h"
#include "FTPSCombatComponent.generated.h"

class UFTPSWeaponData;
class AFTPSWeapon;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReserveAmmoChanged, int32, RoundsInReserve, int32, RoundsInWeapon, UMaterialInterface*, WeaponIconMaterial);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRoundFired, int32, RoundsCurrent, int32, RoundsMax, int32, RoundsInReserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReticleChanged, UMaterialInstanceDynamic*, ReticleDynMatInst, const FReticleParams&, ReticleParams, bool, bCurrentlyTargetingPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAmmoCounterChanged, UMaterialInstanceDynamic*, AmmoCounterDynMatInst, int32, RoundsCurrent, int32, RoundsMax);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimingStatusChanged, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingPlayerStatusChanged, bool, bTargetingPlayer);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FTPSHOOTER_API UFTPSCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFTPSCombatComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// Cycle to the next weapon in the inventory
	void Initiate_CycleWeapon();
	void Initiate_FireWeapon_Pressed();
	void Initiate_FireWeapon_Released();
	void Initiate_ReloadWeapon();
	void Initiate_Aim_Pressed();
	void Initiate_Aim_Released();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<UFTPSWeaponData> WeaponData;

	UFUNCTION(BlueprintPure, Category = "FPS|Combat")
	static UFTPSCombatComponent* FindCombatComponent(const AActor* Actor)
	{
		return IsValid(Actor) ? Actor->FindComponentByClass<UFTPSCombatComponent>() : nullptr;
	}

	UPROPERTY(BlueprintAssignable)
	FReserveAmmoChanged OnCurrentReserveAmmoChanged;

	UPROPERTY(BlueprintAssignable)
	FRoundFired OnRoundFired;

	UPROPERTY(BlueprintAssignable)
	FReticleChanged OnReticleChanged;

	UPROPERTY(BlueprintAssignable)
	FAmmoCounterChanged OnAmmoCounterChanged;

	UPROPERTY(BlueprintAssignable)
	FAimingStatusChanged OnAimingStatusChanged;

	UPROPERTY(BlueprintAssignable)
	FTargetingPlayerStatusChanged OnTargetingPlayerStatusChanged;

	UPROPERTY(BlueprintReadOnly, Category = "FPS|Combat")
	bool bHitPlayer = false;

	void Equip(AFTPSWeapon* Weapon);
	void SpawnInventory();
	void DestroyInventory();
	AFTPSWeapon* GetCurrentWeapon() const { return CurrentWeapon; }
	int32 GetReserveAmmo() const { return ReserveAmmo; }
	void AddReserveAmmo(int32 AmmoAmount) { ReserveAmmo += FMath::Max(0, AmmoAmount); }

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Aiming, Category = "FPS|Combat")
	bool bAiming = false;

protected:
	AFTPSWeapon* SpawnWeapon(TSubclassOf<AFTPSWeapon> WeaponClass) const;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AFTPSWeapon> CurrentWeapon;

private:
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapon")
	TSubclassOf<AFTPSWeapon> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapon")
	TArray<TSubclassOf<AFTPSWeapon>> DefaultWeaponClasses;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapon")
	int32 ReserveAmmo = 90;

	UFUNCTION()
	void OnRep_CurrentWeapon(AFTPSWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_Aiming();

	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);

	void Local_Aim(bool bPressed);

	UPROPERTY(Transient, Replicated)
	TArray<AFTPSWeapon*> Inventory;
};
