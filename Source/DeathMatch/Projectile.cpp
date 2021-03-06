// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/DamageType.h"
#include "Sound/SoundCue.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RootComponent"));
	BoxComponent->SetBoxExtent(FVector(8,1,1));
	BoxComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = BoxComponent;	

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/CustomMeshes/Lasser.Lasser"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(RootComponent);

	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(2, 2, 2));
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultExplosionEffect(TEXT("/Game/StarterContent/Particles/LaserSparks.LaserSparks"));
	if (DefaultExplosionEffect.Succeeded())
	{
		ExplosionEffect = DefaultExplosionEffect.Object;
	}

	LaserHitSound = CreateDefaultSubobject<USoundCue>(TEXT("MeshComp"));

	static ConstructorHelpers::FObjectFinder<USoundCue> LaserSound(TEXT("/Game/Sounds/LaserHit_Cue.LaserHit_Cue"));
	
	if (LaserSound.Succeeded())
	{
		LaserHitSound = LaserSound.Object;
	}
	
	//Definition for the Projectile Movement Component.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(BoxComponent);
	ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(7);
	
	if (GetLocalRole() == ROLE_Authority)
	{
		BoxComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnProjectileImpact);
	}
}

void AProjectile::Destroyed()
{
	if (LaserHitSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(),LaserHitSound,GetActorLocation());
	}

	FVector spawnLocation = GetActorLocation();
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	}
}

void AProjectile::OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ( OtherActor )
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, Damage, NormalImpulse, Hit, GetInstigator()->GetController(), this, DamageType);
	}

	Destroy();
}

