// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Vehicle.h"
#include "Camera/CameraComponent.h"
#include "WheeledVehicleMovementComponent.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "FrontVehicleWheel.h"
#include "RearVehicleWheel.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "VehicleGameMode.h"
#include "Kismet/GameplayStatics.h"

AVehicle::AVehicle() {

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/Car/mclaren/mclaren"));
	GetMesh()->SetSkeletalMesh(CarMesh.Object);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeRotation(FRotator(-15.0f,0,0));
	SpringArm->TargetOffset = FVector(0.0f,0.0f,200.0f);
	SpringArm->TargetArmLength = 750.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.0f;

	ExternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ExternalCamera"));
	ExternalCamera->SetupAttachment(SpringArm);

	InternalCameraOrigin = FVector(-16.0f,-23.0f,20.0f);

	InternalCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("InternalCameraBase"));
	InternalCameraBase->SetupAttachment(GetMesh());
	InternalCameraBase->SetRelativeLocation(InternalCameraOrigin);
	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	InternalCamera->SetupAttachment(InternalCameraBase);

	Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W> (GetVehicleMovement());
	check(Vehicle4W->WheelSetups.Num()==4);

	Vehicle4W->WheelSetups[0].WheelClass = UFrontVehicleWheel::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("right_front_bone");

	Vehicle4W->WheelSetups[1].WheelClass = UFrontVehicleWheel::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("left_front_bone");

	Vehicle4W->WheelSetups[2].WheelClass = URearVehicleWheel::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("right_rear_bone");

	Vehicle4W->WheelSetups[3].WheelClass = URearVehicleWheel::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("left_rear_bone");

	bInCarCameraActive = false;
	bHandbraking = false;

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(RootComponent);
	SteerAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("SteerAudioComp"));
	SteerAudioComp->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<USoundCue> ES(TEXT("/Game/Sound/CanEngine_F_midhigh-register_mono_Cue"));
	EngineSound = ES.Object;

	static ConstructorHelpers::FObjectFinder<USoundCue> SS(TEXT("/Game/Sound/Car_skid1_Cue"));
	SteerSound = SS.Object;

	GearSpeedArray = { 0,18,43,62,120,151 };

	VehicleGameMode= Cast<AVehicleGameMode>( UGameplayStatics::GetGameMode(this));
}

void AVehicle::Tick(float DeltaTime)
{
	OnSkid();
	CurrentSpeedOnDisplay =FMath::Clamp( GetVehicleMovement()->GetForwardSpeed()*0.036f*270/140-180,-180.0f,90.0f);
	CurrentGear = GetVehicleMovement()->GetCurrentGear();
}


void AVehicle::BeginPlay()
{
	Super::BeginPlay();
	if (EngineSound->IsValidLowLevelFast())
	{
		EngineSound->PitchMultiplier = 0.1f;
		AudioComp->SetSound(EngineSound);
		
	}
	if (SteerSound->IsValidLowLevelFast())
	{
		SteerAudioComp->SetSound(SteerSound);
	}
}

void AVehicle::MoveForward(float Value)
{
	if (VehicleGameMode->CurrentState == EGameState::EPlaying)
	{
		GetVehicleMovementComponent()->SetThrottleInput(Value);
		PlayEngineSound();
	}

}

void AVehicle::MoveRight(float Value)
{
	if (VehicleGameMode->CurrentState == EGameState::EPlaying)
	{
		FrontSteerAngle = Value;
		GetVehicleMovementComponent()->SetSteeringInput(Value);
	}
}

void AVehicle::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
	bHandbraking = true;
}

void AVehicle::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
	bHandbraking = false;
}

void AVehicle::OnToggleCamera() {
	if (bInCarCameraActive) {
		ExternalCamera->Activate();
		InternalCamera->Deactivate();
		bInCarCameraActive = false;
	}
	else {
		InternalCamera->Activate();
		ExternalCamera->Deactivate();
		bInCarCameraActive = true;
	}
}



void AVehicle::PlayEngineSound()
{
	
	int Index = 0;
	for (int i = 0; i < GearSpeedArray.Num()-2; i++) {
		if (GetVehicleMovement()->GetForwardSpeed()*0.036f >= GearSpeedArray[i]) {
			Index = i;
		}
	}
	int MinSpeed = GearSpeedArray[Index];
	int MaxSpeed = GearSpeedArray[Index + 1];

	EngineSound->PitchMultiplier= 0.1f+ (FMath::Abs(GetVehicleMovement()->GetForwardSpeed())*0.036f - MinSpeed) / (MaxSpeed - MinSpeed)*0.8f;

	//EngineSound->PitchMultiplier = 0.2f + GetVehicleMovement()->GetForwardSpeed()*0.036f / 152.0f;
	if (!AudioComp->IsPlaying()) {
		AudioComp->Play();
	}
}

void AVehicle::OnSkid()
{
	if (GetVehicleMovement()->GetForwardSpeed()*0.036f > 40&&FMath::Abs( Vehicle4W->Wheels[0]->GetSteerAngle())>20.0f ||
		GetVehicleMovement()->GetForwardSpeed()*0.036f > 20 && FMath::Abs(Vehicle4W->Wheels[0]->GetSteerAngle())>10.0f&&bHandbraking) {
		
		if (!SteerAudioComp->IsPlaying()) {
			SteerAudioComp->Play();
		}
		UE_LOG(LogTemp, Warning, TEXT("OnSkid"));
	}
	else {
		SteerAudioComp->Stop();
	}
}


void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward",this,&AVehicle::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehicle::MoveRight);

	PlayerInputComponent->BindAction("Handbrake",IE_Pressed,this,&AVehicle::OnHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &AVehicle::OnHandbrakeReleased);


	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &AVehicle::OnToggleCamera);
}


