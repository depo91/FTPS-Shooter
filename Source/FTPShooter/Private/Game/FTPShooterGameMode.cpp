// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FTPShooterGameMode.h"
#include "Character/FTPShooterCharacter.h"
#include "Game/ShooterGameStateBase.h"
#include "Player/ShooterPlayerController.h"
#include "Player/ShooterPlayerState.h"

AFTPShooterGameMode::AFTPShooterGameMode()
{
	DefaultPawnClass = AFTPShooterCharacter::StaticClass();
	PlayerControllerClass = AShooterPlayerController::StaticClass();
	PlayerStateClass = AShooterPlayerState::StaticClass();
	GameStateClass = AShooterGameStateBase::StaticClass();
}
