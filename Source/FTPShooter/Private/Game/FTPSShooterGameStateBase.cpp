// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/ShooterGameStateBase.h"

#include "Player/ShooterPlayerState.h"

bool AShooterGameStateBase::HasFirstBloodBeenHad() const
{
	return bFirstBloodBeenHad;
}

AShooterPlayerState* AShooterGameStateBase::GetSoleLeader() const
{
	return SoleLeader;
}

bool AShooterGameStateBase::IsTiedForTheLead(const AShooterPlayerState* PlayerState) const
{
	return IsValid(PlayerState) && CurrentLeaders.Contains(const_cast<AShooterPlayerState*>(PlayerState));
}

void AShooterGameStateBase::UpdateLeader()
{
	CurrentLeaders.Reset();
	SoleLeader = nullptr;

	int32 BestScore = TNumericLimits<int32>::Min();

	for (APlayerState* BasePlayerState : PlayerArray)
	{
		AShooterPlayerState* ShooterPlayerState = Cast<AShooterPlayerState>(BasePlayerState);
		if (!IsValid(ShooterPlayerState))
		{
			continue;
		}

		const int32 PlayerScore = ShooterPlayerState->GetScoredElims();
		if (PlayerScore > BestScore)
		{
			BestScore = PlayerScore;
			CurrentLeaders.Reset();
			CurrentLeaders.Add(ShooterPlayerState);
		}
		else if (PlayerScore == BestScore)
		{
			CurrentLeaders.Add(ShooterPlayerState);
		}
	}

	if (CurrentLeaders.Num() == 1)
	{
		SoleLeader = CurrentLeaders[0];
		SoleLeader->IsNowWinner();
	}

	if (!bFirstBloodBeenHad && BestScore > 0)
	{
		bFirstBloodBeenHad = true;
	}
}
