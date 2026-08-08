// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FTPShooterGameMode.h"
#include "Character/FTPShooterCharacter.h"
#include "Game/FTPSShooterGameStateBase.h"
#include "Player/FTPSShooterPlayerController.h"
#include "Player/FTPSShooterPlayerState.h"

AFTPShooterGameMode::AFTPShooterGameMode()
{
	DefaultPawnClass = AFTPShooterCharacter::StaticClass();
	PlayerControllerClass = AFTPSShooterPlayerController::StaticClass();
	PlayerStateClass = AFTPSShooterPlayerState::StaticClass();
	GameStateClass = AFTPSShooterGameStateBase::StaticClass();
}
