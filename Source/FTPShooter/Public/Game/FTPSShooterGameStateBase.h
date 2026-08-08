// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameStateBase.generated.h"

class AShooterPlayerState;

UCLASS()
class FTPSHOOTER_API AShooterGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	bool HasFirstBloodBeenHad() const;
	AShooterPlayerState* GetSoleLeader() const;
	bool IsTiedForTheLead(const AShooterPlayerState* PlayerState) const;
	void UpdateLeader();

private:
	UPROPERTY()
	TArray<TObjectPtr<AShooterPlayerState>> CurrentLeaders;

	UPROPERTY()
	TObjectPtr<AShooterPlayerState> SoleLeader;

	bool bFirstBloodBeenHad = false;
};
