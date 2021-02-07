// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "SHealthComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"


static int32 DebugExplosiveBarrelDrawing = 0;
FAutoConsoleVariableRef CVARDebugExplosiveBarrelDrawing
(
	TEXT("COOP.DebugExplosiveBarrels"), 
	DebugExplosiveBarrelDrawing, 
	TEXT("Draw debug lines for explosive barrels."),
	ECVF_Cheat
);

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	MeshComp->SetSimulatePhysics(true);
	// Set to physics body to let radial component affect us(eg. when a nearby barrel explodes)
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	BlastComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("BlastComp"));
	BlastComp->SetupAttachment(MeshComp);
	BlastComp->Radius = 100.f;	
	BlastComp->bImpulseVelChange = true;
	BlastComp->bAutoActivate = false;
	BlastComp->bIgnoreOwningActor = true;
	BlastComp->DestructibleDamage = 200.f;

	ExplosionImpulse = 400;
	ExplosionDamage = 400.f;
	ExplosionRadius = 200.f;

	SetReplicates(true);
	SetReplicatingMovement(true);
}


void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta,
 										const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) 
{
	if(bExploded)
	{
		return;
	}

	if (Health <= 0.f)
	{
		bExploded = true;
		OnRep_Exploded();

		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Barrel Explodes")));

		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		BlastComp->FireImpulse();

		if(HasAuthority())
		{
			TArray<AActor*> IgnoreActors;
			IgnoreActors.Add(this);

			// Apply damage
			UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoreActors, this, GetInstigatorController(), true);

			// Debug
			if (DebugExplosiveBarrelDrawing > 0)
			{
				DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Blue, false, 4.f, (uint8)'\000', 3.f);
			}

			SetLifeSpan(5.f);
		}
	}
}


void ASExplosiveBarrel::OnRep_Exploded() 
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeFX, GetActorLocation(), FRotator::ZeroRotator, ((FVector)((2.0F))), true, EPSCPoolMethod::None, false);
	MeshComp->SetMaterial(0, ExplodedMaterial);
}


void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(ASExplosiveBarrel, bExploded); // Replicates the Variable to every machine
}

