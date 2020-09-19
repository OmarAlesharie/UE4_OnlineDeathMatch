// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"


#include "DrawDebugHelpers.h"
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

	//NetUpdateFrequency = 66.0f;
	//MinNetUpdateFrequency = 33.0f;

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
	
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 30000);

		FVector TracerEndPoint = TraceEnd;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECollisionChannel::ECC_GameTraceChannel1, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			TracerEndPoint = Hit.ImpactPoint;
		}

		FVector ShotDirection = (TracerEndPoint - MuzzleLocation).GetSafeNormal();

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = this->GetInstigator();
		spawnParameters.Owner = this->GetOwner();

		AProjectile* spawnedProjectile = GetWorld()->SpawnActor<AProjectile>(MuzzleLocation, ShotDirection.Rotation(), spawnParameters);
	}
	
	
}

bool AWeapon::HandleFire_Validate()
{
	return true;
}

