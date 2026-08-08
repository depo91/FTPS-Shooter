// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FTPSShooterGameStateBase.generated.h"

class AFTPSShooterPlayerState;

UCLASS()
class FTPSHOOTER_API AFTPSShooterGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	bool HasFirstBloodBeenHad() const;
	AFTPSShooterPlayerState* GetSoleLeader() const;
	bool IsTiedForTheLead(const AFTPSShooterPlayerState* PlayerState) const;
	void UpdateLeader();

private:
	UPROPERTY()
	TArray<TObjectPtr<AFTPSShooterPlayerState>> CurrentLeaders;

	UPROPERTY()
	TObjectPtr<AFTPSShooterPlayerState> SoleLeader;

	bool bFirstBloodBeenHad = false;
};
