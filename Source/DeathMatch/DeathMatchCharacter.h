// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DeathMatchCharacter.generated.h"

class AWeapon;
class UHealthComponent;

UCLASS(config=Game)
class ADeathMatchCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UFUNCTION()
    void OnHealthChanged(UHealthComponent* InHealthComp,float Health, float HealthDelta, const class UDamageType* DamageType,class AController* InstigatedBy, AActor* DamageCauser);
	
public:
	ADeathMatchCharacter();
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UFUNCTION(BlueprintCallable)
	FRotator GetOffsetAxis();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conponents")
	UHealthComponent* HealthComp;
	
	bool bWantsToZoom;
	
	UPROPERTY(EditDefaultsOnly, Category="Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category="Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;
	
	float DefaultFOV;

	void BeginZoom();
	
	void EndZoom();
	
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	void BeginCrouch();
	void EndCrouch();

	UFUNCTION(BlueprintImplementableEvent, Category="WeaponZoom")
    void ZoomStart();

	UFUNCTION(BlueprintImplementableEvent, Category="WeaponZoom")
    void ZoomEnd();

	UPROPERTY(Replicated)
	FRotator OffsetAxis;

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category="ServerFunctions")
	void ServerSetOffsetAxis();

	UPROPERTY(Replicated)
	AWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	// Pawn died previously
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Player")
	bool bDied;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

    void StartFire();


public:
	virtual void Tick(float DeltaSeconds) override;
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual FVector GetPawnViewLocation() const override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Replicated)
	bool bCanFire;
};

