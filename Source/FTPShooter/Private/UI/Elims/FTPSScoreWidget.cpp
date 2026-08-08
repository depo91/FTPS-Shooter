// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/Elims/FTPSScoreWidget.h"

#include "Components/TextBlock.h"
#include "Player/FTPSShooterPlayerController.h"
#include "Player/FTPSShooterPlayerState.h"

void UFTPSScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	AFTPSShooterPlayerState* PS = GetPlayerState();
	if (IsValid(PS))
	{
		PS->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);
	}
	else
	{
		AFTPSShooterPlayerController* PC = Cast<AFTPSShooterPlayerController>(GetOwningPlayer());
		if (IsValid(PC))
		{
			PC->OnPlayerStateReplicated.AddUniqueDynamic(this, &ThisClass::OnPlayerStateReplicated);
		}
	}
}

void UFTPSScoreWidget::OnPlayerStateReplicated()
{
	AFTPSShooterPlayerState* PS = GetPlayerState();
	if (IsValid(PS))
	{
		PS->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);
		OnScoreChanged(PS->GetScoredElims());
	}
	
	AFTPSShooterPlayerController* PC = Cast<AFTPSShooterPlayerController>(GetOwningPlayer());
	if (IsValid(PC))
	{
		PC->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
}

AFTPSShooterPlayerState* UFTPSScoreWidget::GetPlayerState() const
{
	APlayerController* PC = GetOwningPlayer();
	if (IsValid(PC))
	{
		return PC->GetPlayerState<AFTPSShooterPlayerState>();
	}
	
	return nullptr;
}

void UFTPSScoreWidget::OnScoreChanged(int32 Score)
{
	if (IsValid(Text_Score))
	{
		Text_Score->SetText(FText::AsNumber(Score));
	}
}
