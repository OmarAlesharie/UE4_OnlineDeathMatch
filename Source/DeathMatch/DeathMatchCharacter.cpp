// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeathMatchCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "HealthComponent.h"

//////////////////////////////////////////////////////////////////////////
// ADeathMatchCharacter


ADeathMatchCharacter::ADeathMatchCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetRelativeLocation(FVector(0.0f,80.0f,90.0f));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	
	WeaponAttachSocketName = "WeaponSocket";

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADeathMatchCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ADeathMatchCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ADeathMatchCharacter::EndCrouch);

	PlayerInputComponent->BindAxis("MoveForward", this, &ADeathMatchCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADeathMatchCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ADeathMatchCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADeathMatchCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ADeathMatchCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ADeathMatchCharacter::TouchStopped);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ADeathMatchCharacter::StartFire);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ADeathMatchCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Zoom", EInputEvent::IE_Pressed, this, &ADeathMatchCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", EInputEvent::IE_Released, this, &ADeathMatchCharacter::EndZoom);
}

void ADeathMatchCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = FollowCamera->FieldOfView;

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	HealthComp->OnHealthChanged.AddDynamic(this, &ADeathMatchCharacter::OnHealthChanged);
	
	if (HasAuthority())
	{
		// Spawn a default weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;

		CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}
}

void ADeathMatchCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ADeathMatchCharacter::Tick(float DeltaSeconds)
{
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaSeconds, ZoomInterpSpeed);

	FollowCamera->SetFieldOfView(NewFOV);
}

FVector ADeathMatchCharacter::GetPawnViewLocation() const
{
	if (FollowCamera)
	{
		return FollowCamera->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ADeathMatchCharacter::BeginZoom()
{
	bWantsToZoom = true;
	ZoomStart();
}

void ADeathMatchCharacter::EndZoom()
{
	bWantsToZoom = false;
	ZoomEnd();
}

void ADeathMatchCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ADeathMatchCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ADeathMatchCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ADeathMatchCharacter::BeginCrouch()
{
	Crouch();
}

void ADeathMatchCharacter::EndCrouch()
{
	UnCrouch();
}

void ADeathMatchCharacter::ServerSetOffsetAxis_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		OffsetAxis = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(),GetActorRotation());
	}
}

FRotator ADeathMatchCharacter::GetOffsetAxis()
{
	return OffsetAxis;
}

bool ADeathMatchCharacter::ServerSetOffsetAxis_Validate()
{
	return true;
}

void ADeathMatchCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ADeathMatchCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ADeathMatchCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ADeathMatchCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ADeathMatchCharacter::OnHealthChanged(UHealthComponent* InHealthComp, float Health, float HealthDelta,
    const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		// Die!
		bDied = true;
		
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		CurrentWeapon->SetLifeSpan(12);

		SetLifeSpan(10.0);
	}
}

void ADeathMatchCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeathMatchCharacter, CurrentWeapon);
	DOREPLIFETIME(ADeathMatchCharacter, OffsetAxis);
	DOREPLIFETIME(ADeathMatchCharacter, bDied);
}