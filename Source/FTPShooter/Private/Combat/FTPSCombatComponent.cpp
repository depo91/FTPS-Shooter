// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FTPSCombatComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/FTPSPlayerInterface.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/FTPSWeapon.h"

UFTPSCombatComponent::UFTPSCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UFTPSCombatComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UFTPSCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFTPSCombatComponent, CurrentWeapon);
	DOREPLIFETIME(UFTPSCombatComponent, Inventory);
	DOREPLIFETIME_CONDITION(UFTPSCombatComponent, bAiming, COND_SkipOwner);
}

void UFTPSCombatComponent::Initiate_CycleWeapon()
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

void UFTPSCombatComponent::Initiate_FireWeapon_Pressed()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_FireWeapon_Pressed"), false);

	if (CurrentWeapon && CurrentWeapon->Ammo > 0)
	{
		CurrentWeapon->Ammo = FMath::Clamp(CurrentWeapon->Ammo - 1, 0, CurrentWeapon->MagCapacity);
		OnRoundFired.Broadcast(CurrentWeapon->Ammo, CurrentWeapon->MagCapacity, ReserveAmmo);
	}
}

void UFTPSCombatComponent::Initiate_FireWeapon_Released()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_FireWeapon_Released"), false);
}

void UFTPSCombatComponent::Initiate_ReloadWeapon()
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

void UFTPSCombatComponent::Initiate_Aim_Pressed()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_Aim_Pressed"), false);
	Local_Aim(true);

	if (AActor* OwnerActor = GetOwner(); IsValid(OwnerActor) && !OwnerActor->HasAuthority())
	{
		Server_Aim(true);
	}
}

void UFTPSCombatComponent::Initiate_Aim_Released()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Initiate_Aim_Released"), false);
	Local_Aim(false);

	if (AActor* OwnerActor = GetOwner(); IsValid(OwnerActor) && !OwnerActor->HasAuthority())
	{
		Server_Aim(false);
	}
}

void UFTPSCombatComponent::Server_Aim_Implementation(bool bPressed)
{
	Local_Aim(bPressed);
}

void UFTPSCombatComponent::Local_Aim(bool bPressed)
{
	bAiming = bPressed;
	OnAimingStatusChanged.Broadcast(bAiming);
}

void UFTPSCombatComponent::Equip(AFTPSWeapon* Weapon)
{
	if (!IsValid(Weapon))
	{
		return;
	}

	AFTPSWeapon* LastWeapon = CurrentWeapon;
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

void UFTPSCombatComponent::SpawnInventory()
{
	if (!GetOwner() || GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	for (const TSubclassOf<AFTPSWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		if (AFTPSWeapon* Weapon = SpawnWeapon(WeaponClass))
		{
			Inventory.AddUnique(Weapon);
		}
	}

	if (*DefaultWeaponClass)
	{
		if (AFTPSWeapon* Weapon = SpawnWeapon(DefaultWeaponClass))
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

void UFTPSCombatComponent::DestroyInventory()
{
	for (AFTPSWeapon* Weapon : Inventory)
	{
		if (IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}

	Inventory.Reset();
	CurrentWeapon = nullptr;
}

void UFTPSCombatComponent::OnRep_CurrentWeapon(AFTPSWeapon* LastWeapon)
{
	if (CurrentWeapon)
	{
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

	if (AActor* OwnerActor = GetOwner();
		IsValid(OwnerActor) && OwnerActor->GetClass()->ImplementsInterface(UFTPSPlayerInterface::StaticClass()))
	{
		IFTPSPlayerInterface::Execute_WeaponReplicated(OwnerActor);
	}
}

void UFTPSCombatComponent::OnRep_Aiming()
{
	OnAimingStatusChanged.Broadcast(bAiming);
}

AFTPSWeapon* UFTPSCombatComponent::SpawnWeapon(TSubclassOf<AFTPSWeapon> WeaponClass) const
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

	return GetWorld()->SpawnActor<AFTPSWeapon>(WeaponClass, SpawnParams);
}
