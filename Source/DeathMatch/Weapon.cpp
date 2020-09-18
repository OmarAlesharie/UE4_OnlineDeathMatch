// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
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

	BulletSpread = 2.0f;
	RateOfFire = 600;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
	
}

void AWeapon::Fire()
{
	
}

void AWeapon::StartFire()
{
	GEngine->AddOnScreenDebugMessage(-1,1, FColor::Red, TEXT("Fire"));
}

void AWeapon::StopFire()
{
	
}

void AWeapon::ServerFire_Implementation()
{
	
}

bool AWeapon::ServerFire_Validate()
{
	return true;
}

