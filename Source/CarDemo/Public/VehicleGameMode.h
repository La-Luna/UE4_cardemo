// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VehicleGameMode.generated.h"

class USoundCue;

enum class EGameState :short {
	EWait,
	EPlaying,
};

/**
 * 
 */
UCLASS()
class CARDEMO_API AVehicleGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	AVehicleGameMode();

	FTimerHandle TimerHandle_TimeCount;

	void CutTime();

	UPROPERTY(BlueprintReadOnly,Category="UI")
	int CountTime;

	USoundCue* CutTimeSoundCue;
	USoundCue* BeginPlaySoundCue;


public:
	virtual void StartPlay() override;
	
	EGameState CurrentState;
};
