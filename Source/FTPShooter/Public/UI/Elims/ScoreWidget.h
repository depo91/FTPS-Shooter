// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

class AShooterPlayerState;
class UTextBlock;

UCLASS()
class FTPSHOOTER_API UScoreWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Score;
	
private:
	
	AShooterPlayerState* GetPlayerState() const;
	
	UFUNCTION()
	void OnScoreChanged(int32 Score);
	
	UFUNCTION()
	void OnPlayerStateReplicated();
};


