// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FTPSShooterGameStateBase.h"

#include "Player/FTPSShooterPlayerState.h"

bool AFTPSShooterGameStateBase::HasFirstBloodBeenHad() const
{
	return bFirstBloodBeenHad;
}

AFTPSShooterPlayerState* AFTPSShooterGameStateBase::GetSoleLeader() const
{
	return SoleLeader;
}

bool AFTPSShooterGameStateBase::IsTiedForTheLead(const AFTPSShooterPlayerState* PlayerState) const
{
	return IsValid(PlayerState) && CurrentLeaders.Contains(const_cast<AFTPSShooterPlayerState*>(PlayerState));
}

void AFTPSShooterGameStateBase::UpdateLeader()
{
	CurrentLeaders.Reset();
	SoleLeader = nullptr;

	int32 BestScore = TNumericLimits<int32>::Min();

	for (APlayerState* BasePlayerState : PlayerArray)
	{
		AFTPSShooterPlayerState* ShooterPlayerState = Cast<AFTPSShooterPlayerState>(BasePlayerState);
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
