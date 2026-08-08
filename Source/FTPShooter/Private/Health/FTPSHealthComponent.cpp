// Copyright Epic Games, Inc. All Rights Reserved.


#include "Health/FTPSHealthComponent.h"
#include "Net/UnrealNetwork.h"

UFTPSHealthComponent::UFTPSHealthComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	DeathState = EDeathState::NotDead;
	SetIsReplicatedByDefault(true);
	Health = 100.f;
	MaxHealth = 100.f;
}

void UFTPSHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFTPSHealthComponent, DeathState);
	DOREPLIFETIME_CONDITION(UFTPSHealthComponent, Health, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UFTPSHealthComponent, MaxHealth, COND_OwnerOnly);
}

float UFTPSHealthComponent::GetHealthNormalized() const
{
	return (MaxHealth > 0.f) ? (Health / MaxHealth) : 0.f;
}

bool UFTPSHealthComponent::ChangeHealthByAmount(float Amount, AActor* Instigator)
{
	float OldValue = Health;
	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(this, OldValue, Health, Instigator);
	
	if (Health <= 0.f)
	{
		StartDeath();
		return true;
	}
	
	return false;
}

void UFTPSHealthComponent::StartDeath()
{
	if (DeathState != EDeathState::NotDead)
	{
		return;
	}
	
	DeathState = EDeathState::DeathStarted;
	OnDeathStarted.Broadcast();
	GetOwner()->ForceNetUpdate();
}

void UFTPSHealthComponent::ChangeMaxHealthByAmount(float Amount, AActor* Instigator)
{
	float OldValue = MaxHealth;
	MaxHealth += Amount;
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, Instigator);
}

void UFTPSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UFTPSHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	if (DeathState == EDeathState::DeathStarted)
	{
		OnDeathStarted.Broadcast();
	}
}

void UFTPSHealthComponent::OnRep_Health(float OldValue)
{
	OnHealthChanged.Broadcast(this, OldValue, Health, nullptr);
}

void UFTPSHealthComponent::OnRep_MaxHealth(float OldValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, nullptr);
}

