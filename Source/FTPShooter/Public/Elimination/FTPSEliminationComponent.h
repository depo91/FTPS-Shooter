// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FTPSEliminationComponent.generated.h"


class AFTPSShooterGameStateBase;
enum class ESpecialElimType : uint16;
class AFTPSShooterPlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FTPSHOOTER_API UFTPSEliminationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFTPSEliminationComponent();
	
	UFUNCTION()
	void OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal);

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Elimination")
	float SequentialElimInterval;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Elimination")
	int32 ElimsNeededForStreak;
private:
	
	float LastElimTime;
	int32 SequentialElims;
	int32 Streak;
	
	AFTPSShooterPlayerState* GetPlayerStateFromActor(AActor* Actor);
	void ProcessHitOrMiss(bool bHit, AFTPSShooterPlayerState* AttackerPS);
	void ProcessElimination(bool bHeadShot, AFTPSShooterPlayerState* AttackerPS, AFTPSShooterPlayerState* VictimPS);
	void ProcessHeadshot(bool bHeadShot, ESpecialElimType& OutElimType, AFTPSShooterPlayerState* AttackerPS);
	void ProcessSequentialEliminations(ESpecialElimType& OutElimType, AFTPSShooterPlayerState* AttackerPS);
	void ProcessStreaks(ESpecialElimType& OutElimType, AFTPSShooterPlayerState* AttackerPS, AFTPSShooterPlayerState* VictimPS);
	void HandleFirstBlood(AFTPSShooterGameStateBase* GameState, ESpecialElimType& OutElimType, AFTPSShooterPlayerState* AttackerPS);
	void UpdateLeaderStatus(AFTPSShooterGameStateBase* GameState, ESpecialElimType& OutElimType, AFTPSShooterPlayerState* AttackerPS, AFTPSShooterPlayerState* VictimPS);
	bool HasSpecialElimTypes(const ESpecialElimType& SpecialElimType) const;
};
