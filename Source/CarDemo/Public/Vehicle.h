// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicle.h"
#include "Vehicle.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USoundCue;
class UAudioComponent;
class UWheeledVehicleMovementComponent4W;
class AVehicleGameMode;

/**
 * 
 */
UCLASS()
class CARDEMO_API AVehicle : public AWheeledVehicle
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere,Category="CameraComponent")
	UCameraComponent* ExternalCamera;

	UPROPERTY(VisibleAnywhere, Category = "CameraComponent")
		UCameraComponent* InternalCamera;

	UPROPERTY(VisibleAnywhere, Category = "CameraComponent")
	USceneComponent* InternalCameraBase;

	FVector InternalCameraOrigin;

	UPROPERTY(VisibleAnywhere,Category = "CameraComponent")
	USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* EngineSound;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	UAudioComponent* AudioComp;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundCue* SteerSound;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
		UAudioComponent* SteerAudioComp;

	bool bInCarCameraActive;

	bool bHandbraking;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void OnHandbrakePressed();

	void OnHandbrakeReleased();

	void OnToggleCamera();

	virtual void BeginPlay() override;

	void PlayEngineSound();

	void OnSkid();

	TArray<int32> GearSpeedArray;

	UWheeledVehicleMovementComponent4W* Vehicle4W;

	float FrontSteerAngle;

	AVehicleGameMode* VehicleGameMode;

	UPROPERTY(BlueprintReadOnly,Category="UI")
	float CurrentSpeedOnDisplay;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	int CurrentGear;

public:
	AVehicle();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
