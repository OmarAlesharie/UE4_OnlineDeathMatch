// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Projectile.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AWeapon::AWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> WeaponMesh(TEXT("/Game/SciFiWeapDark/Weapons/Darkness_AssaultRifle.Darkness_AssaultRifle"));

	if (WeaponMesh.Succeeded())
	{
		MeshComp->SetSkeletalMesh(WeaponMesh.Object);
	}
	
	MuzzleSocketName = "MuzzleSocket";

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	//Initialize projectile class
	ProjectileClass = AProjectile::StaticClass();
	//Initialize fire rate
	FireRate = 0.25f;
	bIsFiringWeapon = false;

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::StartFire()
{
	if (!bIsFiringWeapon)
	{
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &AWeapon::StopFire, FireRate, false);
		HandleFire();
	}
}

void AWeapon::StopFire()
{
	bIsFiringWeapon = false;
}

void AWeapon::HandleFire_Implementation()
{
	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = this->GetInstigator();
	spawnParameters.Owner = this->GetOwner();

	AProjectile* spawnedProjectile = GetWorld()->SpawnActor<AProjectile>(MuzzleLocation, MuzzleRotation, spawnParameters);
}

bool AWeapon::HandleFire_Validate()
{
	return true;
}

