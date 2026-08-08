// Copyright Epic Games, Inc. All Rights Reserved.


#include "Elimination/FTPSEliminationComponent.h"

#include "Engine/World.h"
#include "Game/FTPSShooterGameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FTPSShooterPlayerState.h"
#include "ShooterTypes/FTPSShooterTypes.h"


UFTPSEliminationComponent::UFTPSEliminationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SequentialElimInterval = 2.f;
	LastElimTime = 0.f;
	SequentialElims = 0;
	Streak = 0;
	ElimsNeededForStreak = 5;
}

void UFTPSEliminationComponent::OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal)
{
	AFTPSShooterPlayerState* AttackerPS = GetPlayerStateFromActor(Attacker);
	if (!IsValid(AttackerPS)) return;
	
	ProcessHitOrMiss(bHit, AttackerPS);
	
	if (!bHit) return; // Early return if it was a miss
	
	AFTPSShooterPlayerState* VictimPS = GetPlayerStateFromActor(Victim);
	if (!IsValid(VictimPS)) return;
	
	if (bLethal)
	{
		ProcessElimination(bHeadShot, AttackerPS, VictimPS);
	}
}

void UFTPSEliminationComponent::ProcessElimination(bool bHeadShot, AFTPSShooterPlayerState* AttackerPS, AFTPSShooterPlayerState* VictimPS)
{
	AttackerPS->AddScoredElim();
	VictimPS->AddDefeat();
	
	ESpecialElimType SpecialElimType{};
	
	ProcessHeadshot(bHeadShot, SpecialElimType, AttackerPS);
	ProcessSequentialEliminations(SpecialElimType, AttackerPS);
	ProcessStreaks(SpecialElimType, AttackerPS, VictimPS);

	AFTPSShooterGameStateBase* GameState = Cast<AFTPSShooterGameStateBase>(UGameplayStatics::GetGameState(AttackerPS));
	if (IsValid(GameState))
	{
		HandleFirstBlood(GameState, SpecialElimType, AttackerPS);
		UpdateLeaderStatus(GameState, SpecialElimType, AttackerPS, VictimPS);
	}
	
	if (HasSpecialElimTypes(SpecialElimType))
	{
		AttackerPS->Client_SpecialElim(SpecialElimType, SequentialElims, Streak, AttackerPS->GetScoredElims());
	}
	else
	{
		AttackerPS->Client_ScoredElim(AttackerPS->GetScoredElims());
	}
}

void UFTPSEliminationComponent::ProcessHeadshot(bool bHeadShot, ESpecialElimType& OutElimType,
	AFTPSShooterPlayerState* AttackerPS)
{
	if (bHeadShot)
	{
		OutElimType |= ESpecialElimType::Headshot;
		AttackerPS->AddHeadShotElim();
	}
}

void UFTPSEliminationComponent::ProcessSequentialEliminations(ESpecialElimType& OutElimType,
	AFTPSShooterPlayerState* AttackerPS)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastElimTime <= SequentialElimInterval)
	{
		++SequentialElims;
	}
	else
	{
		SequentialElims = 1;
	}
	LastElimTime = CurrentTime;
	
	if (SequentialElims > 1)
	{
		OutElimType |= ESpecialElimType::Sequential;
		AttackerPS->AddSequentialElim(SequentialElims);
	}
}

void UFTPSEliminationComponent::ProcessStreaks(ESpecialElimType& OutElimType, AFTPSShooterPlayerState* AttackerPS,
	AFTPSShooterPlayerState* VictimPS)
{
	++Streak;
	if (Streak >= ElimsNeededForStreak)
	{
		OutElimType |= ESpecialElimType::Streak;
		AttackerPS->SetOnStreak(true);
		AttackerPS->UpdateHighestStreak(Streak);
	}
	if (VictimPS->IsOnStreak())
	{
		OutElimType |= ESpecialElimType::Showstopper;
		AttackerPS->AddShowStopperElim();
		VictimPS->SetOnStreak(false);
	}
	if (AttackerPS->GetLastAttacker() == VictimPS)
	{
		OutElimType |= ESpecialElimType::Revenge;
		AttackerPS->AddRevengeElim();
		AttackerPS->SetLastAttacker(nullptr);
	}
	VictimPS->SetLastAttacker(AttackerPS);
}

void UFTPSEliminationComponent::HandleFirstBlood(AFTPSShooterGameStateBase* GameState, ESpecialElimType& OutElimType,
	AFTPSShooterPlayerState* AttackerPS)
{
	if (!GameState->HasFirstBloodBeenHad())
	{
		OutElimType |= ESpecialElimType::FirstBlood;
		AttackerPS->GotFirstBlood();
	}
}

void UFTPSEliminationComponent::UpdateLeaderStatus(AFTPSShooterGameStateBase* GameState, ESpecialElimType& OutElimType,
	AFTPSShooterPlayerState* AttackerPS, AFTPSShooterPlayerState* VictimPS)
{
	AFTPSShooterPlayerState* LastLeader =  GameState->GetSoleLeader();
	const bool bAttackerWasTiedForTheLead = GameState->IsTiedForTheLead(AttackerPS);
	GameState->UpdateLeader();
	if (!bAttackerWasTiedForTheLead && GameState->IsTiedForTheLead(AttackerPS))
	{
		// Attacker was not tied for the lead before and is now tied for the lead
		OutElimType |= ESpecialElimType::TiedTheLeader;
	}
	if (IsValid(LastLeader) && LastLeader != GameState->GetSoleLeader())
	{
		// LastLeader has lost the lead
		LastLeader->Client_LostTheLead();
		
		if (VictimPS == LastLeader)
		{
			OutElimType |= ESpecialElimType::Dethrone;
			AttackerPS->AddDethroneElim();
		}
	}
	
	if (AttackerPS != LastLeader && AttackerPS == GameState->GetSoleLeader())
	{
		OutElimType |= ESpecialElimType::GainedTheLead;
	}
}

bool UFTPSEliminationComponent::HasSpecialElimTypes(const ESpecialElimType& SpecialElimType) const
{
	return static_cast<uint16>(SpecialElimType) != 0;
}

void UFTPSEliminationComponent::ProcessHitOrMiss(bool bHit, AFTPSShooterPlayerState* AttackerPS)
{
	if (bHit)
	{
		AttackerPS->AddHit();
	}
	else
	{
		AttackerPS->AddMiss();
	}
}

AFTPSShooterPlayerState* UFTPSEliminationComponent::GetPlayerStateFromActor(AActor* Actor)
{
	APawn* Pawn = Cast<APawn>(Actor);
	if (IsValid(Pawn))
	{
		return Pawn->GetPlayerState<AFTPSShooterPlayerState>();
	}
	return nullptr;
}



