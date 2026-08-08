// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/FTPSWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/FTPSPlayerInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

AFTPSWeapon::AFTPSWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetHiddenInGame(true);
	SetRootComponent(Mesh1P);

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetupAttachment(Mesh1P);
	Mesh3P->SetHiddenInGame(true);
}

USkeletalMeshComponent* AFTPSWeapon::GetMesh1P() const
{
	return Mesh1P;
}

USkeletalMeshComponent* AFTPSWeapon::GetMesh3P() const
{
	return Mesh3P;
}

void AFTPSWeapon::OnRep_Instigator()
{
	Super::OnRep_Instigator();
	AttachToOwningPawn();
}

void AFTPSWeapon::AttachToOwningPawn() const
{
	APawn* OwningPawn = GetInstigator();
	if (!IsValid(OwningPawn) || !OwningPawn->Implements<UFTPSPlayerInterface>())
	{
		return;
	}

	SetMeshVisibilities(OwningPawn);
	const FName AttachPoint = IFTPSPlayerInterface::Execute_GetWeaponAttachPoint(OwningPawn, WeaponType);
	USkeletalMeshComponent* PawnMesh1P = IFTPSPlayerInterface::Execute_GetMesh1P(OwningPawn);
	USkeletalMeshComponent* PawnMesh3P = IFTPSPlayerInterface::Execute_GetMesh3P(OwningPawn);

	if (PawnMesh1P)
	{
		Mesh1P->AttachToComponent(PawnMesh1P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
	}

	if (PawnMesh3P)
	{
		Mesh3P->AttachToComponent(PawnMesh3P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
	}
}

void AFTPSWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (ReticleMaterial)
	{
		ReticleDynamicMaterialInstance = UMaterialInstanceDynamic::Create(ReticleMaterial, this);
	}

	if (AmmoCounterMaterial)
	{
		AmmoCounterDynamicMaterialInstance = UMaterialInstanceDynamic::Create(AmmoCounterMaterial, this);
	}
}

void AFTPSWeapon::SetMeshVisibilities(APawn* OwningPawn) const
{
	if (OwningPawn->IsLocallyControlled())
	{
		Mesh1P->SetHiddenInGame(false);
		Mesh3P->SetHiddenInGame(true);
	}
	else
	{
		Mesh1P->SetHiddenInGame(true);
		Mesh3P->SetHiddenInGame(false);
	}
}
