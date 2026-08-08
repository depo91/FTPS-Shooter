// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ShooterTypes/FTPSShooterTypes.h"
#include "FTPSWeapon.generated.h"

class USkeletalMeshComponent;
class APawn;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class FTPSHOOTER_API AFTPSWeapon : public AActor
{
	GENERATED_BODY()

public:
	AFTPSWeapon();
	virtual void OnRep_Instigator() override;

	USkeletalMeshComponent* GetMesh1P() const;
	USkeletalMeshComponent* GetMesh3P() const;
	UMaterialInstanceDynamic* GetReticleDynamicMaterialInstance() const { return ReticleDynamicMaterialInstance; }
	UMaterialInstanceDynamic* GetAmmoCounterDynamicMaterialInstance() const { return AmmoCounterDynamicMaterialInstance; }
	void AttachToOwningPawn() const;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|WeaponType")
	FGameplayTag WeaponType;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	int32 Ammo = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	int32 MagCapacity = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<UMaterialInterface> WeaponIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	FReticleParams ReticleParams;

private:
	// Weapon Mesh: 1st person view
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh1P;

	// Weapon Mesh: 3rd person view
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh3P;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapon")
	TObjectPtr<UMaterialInterface> ReticleMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapon")
	TObjectPtr<UMaterialInterface> AmmoCounterMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ReticleDynamicMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> AmmoCounterDynamicMaterialInstance;

	void SetMeshVisibilities(APawn* OwningPawn) const;
};
