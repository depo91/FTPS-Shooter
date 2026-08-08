// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/CombatComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCombatComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, Inventory);
}

void UCombatComponent::Initiate_CycleWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_CycleWeapon"), false);

	if (Inventory.Num() < 2)
	{
		return;
	}

	const int32 CurrentIndex = Inventory.IndexOfByKey(CurrentWeapon);
	const int32 NextIndex = CurrentIndex == INDEX_NONE ? 0 : (CurrentIndex + 1) % Inventory.Num();
	Equip(Inventory[NextIndex]);
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_FireWeapon_Pressed"), false);

	if (CurrentWeapon && CurrentWeapon->Ammo > 0)
	{
		CurrentWeapon->Ammo = FMath::Clamp(CurrentWeapon->Ammo - 1, 0, CurrentWeapon->MagCapacity);
		OnRoundFired.Broadcast(CurrentWeapon->Ammo, CurrentWeapon->MagCapacity, ReserveAmmo);
	}
}

void UCombatComponent::Initiate_FireWeapon_Released()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_FireWeapon_Released"), false);
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_ReloadWeapon"), false);

	if (!CurrentWeapon || ReserveAmmo <= 0)
	{
		return;
	}

	const int32 MissingRounds = FMath::Max(0, CurrentWeapon->MagCapacity - CurrentWeapon->Ammo);
	const int32 RoundsToLoad = FMath::Min(MissingRounds, ReserveAmmo);

	CurrentWeapon->Ammo += RoundsToLoad;
	ReserveAmmo -= RoundsToLoad;

	OnCurrentReserveAmmoChanged.Broadcast(ReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
}

void UCombatComponent::Initiate_Aim_Pressed()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_Aim_Pressed"), false);
	OnAimingStatusChanged.Broadcast(true);
}

void UCombatComponent::Initiate_Aim_Released()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_Aim_Released"), false);
	OnAimingStatusChanged.Broadcast(false);
}

void UCombatComponent::Equip(AWeapon* Weapon)
{
	if (!IsValid(Weapon))
	{
		return;
	}

	AWeapon* LastWeapon = CurrentWeapon;
	CurrentWeapon = Weapon;
	CurrentWeapon->AttachToOwningPawn();

	if (LastWeapon && LastWeapon != CurrentWeapon)
	{
		LastWeapon->GetMesh1P()->SetHiddenInGame(true);
		LastWeapon->GetMesh3P()->SetHiddenInGame(true);
	}

	OnReticleChanged.Broadcast(CurrentWeapon->GetReticleDynamicMaterialInstance(), CurrentWeapon->ReticleParams, bHitPlayer);
	OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynamicMaterialInstance(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
	OnCurrentReserveAmmoChanged.Broadcast(ReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	OnTargetingPlayerStatusChanged.Broadcast(bHitPlayer);
}

void UCombatComponent::SpawnInventory()
{
	if (!GetOwner() || GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	for (const TSubclassOf<AWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		if (AWeapon* Weapon = SpawnWeapon(WeaponClass))
		{
			Inventory.AddUnique(Weapon);
		}
	}

	if (*DefaultWeaponClass)
	{
		if (AWeapon* Weapon = SpawnWeapon(DefaultWeaponClass))
		{
			Inventory.AddUnique(Weapon);
		}
	}

	if (Inventory.Num() > 0)
	{
		Inventory[0]->AttachToOwningPawn();
		Equip(Inventory[0]);
	}
}

void UCombatComponent::DestroyInventory()
{
	for (AWeapon* Weapon : Inventory)
	{
		if (IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}

	Inventory.Reset();
	CurrentWeapon = nullptr;
}

AWeapon* UCombatComponent::SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const
{
	AActor* OwningActor = GetOwner();
	if (!IsValid(OwningActor) || !*WeaponClass)
	{
		return nullptr;
	}

	if (OwningActor->GetLocalRole() < ROLE_Authority)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningActor;
	SpawnParams.Instigator = Cast<APawn>(OwningActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnParams);
}
