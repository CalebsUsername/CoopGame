// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "SCharacter.h"
#include "Net/UnrealNetwork.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing
(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw Debug Lines For Weapons"),
	ECVF_Cheat
);


// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = TEXT("MuzzleSocket");
	TracerTargetName = TEXT("Target");

	MaxRange = 1000.f;
	BaseDamage = 10.f;
	FireRate = 400.f;
	BulletSpread = .5f;
	// BloomIncrease = .1f;
	BaseMagazineSize = 35.f;
	CurrentMagSize = BaseMagazineSize;

	SetReplicates(true);

	MinNetUpdateFrequency = 33.f;
}


void ASWeapon::BeginPlay() 
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / FireRate;
}


void ASWeapon::StartFire() 
{
	float FirstDelay = FMath::Max( LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds + .2f, 0.f);
	GetWorldTimerManager().SetTimer(TH_FireRate, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}


void ASWeapon::EndFire() 
{
	GetWorldTimerManager().ClearTimer(TH_FireRate);
}


// On pressing the fire button/trigger...
// Check line trace to see if an actor was hit, and apply point damage if successful
void ASWeapon::Fire() 
{
	if (!HasAuthority())
	{
		ServerFire();
	}

	// Exit function if player is out of ammo
	// Play Empty Magazine Effects
	if(MuzzleEffect && CurrentMagSize == 0)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
		return;
	}

	FHitResult Hit;
	FVector ShotDirection;

	EPhysicalSurface SurfaceType = SurfaceType_Default;

	bool WasHit = GunTrace(Hit, ShotDirection);
	AActor* MyOwner = GetOwner();

	if (WasHit)
	{
		if(CurrentMagSize != 0){CurrentMagSize -= 1.f;}
		AActor* HitActor = Hit.GetActor();

		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		float DamageApplied = BaseDamage;
		if (SurfaceType == SURFACE_FLESHVULNERABLE)
		{
			DamageApplied = BaseDamage * 2.5;
		}

		if (MyOwner)
		{
		UGameplayStatics::ApplyPointDamage(HitActor, DamageApplied, ShotDirection, Hit, GetOwnerController(), MyOwner, DamageType);
		}
			
		TracerEnd = Hit.ImpactPoint;
	
		PlayImpactEffects(SurfaceType, Hit.ImpactPoint);
		PlayWeaponEffects(TracerEnd);
		
		LastFireTime = GetWorld()->TimeSeconds;

		if (HasAuthority())		
		{
			HitScanTrace.TraceTo = TracerEnd;
			HitScanTrace.SurfaceType = SurfaceType;
			HitScanTrace.a++;
		}
	}

	// Still affect weapon attributes even if nothing was hit with GunTrace
	else
	{
		if(GetOwnerController())
		{
			if (HasAuthority())		
			{
				HitScanTrace.TraceTo = TracerEnd;
				HitScanTrace.a++;
			}
			
			if(CurrentMagSize != 0){CurrentMagSize -= 1.f;}

			PlayWeaponEffects(TracerEnd);
			LastFireTime = GetWorld()->TimeSeconds;
		}
	}
}


void ASWeapon::OnReload() 
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerReload();
	}

	CurrentMagSize = BaseMagazineSize;
}


void ASWeapon::ServerReload_Implementation() 
{
	OnReload();
}


bool ASWeapon::ServerReload_Validate() 
{
	return true;
}


void ASWeapon::ServerFire_Implementation() 
{
	Fire();
}


bool ASWeapon::ServerFire_Validate() 
{
	return true;
}


// LineTrace from camera viewpoint to FVector MaxRange
bool ASWeapon::GunTrace(FHitResult& Hit, FVector ShotDirection) 
{
	AController* OwnerController = GetOwnerController();
	if(OwnerController == nullptr) {return false;}

	FVector Location;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Location, Rotation);

	ShotDirection = Rotation.Vector();

	// Bullet spread
	// BulletSpread += BloomIncrease;
	float HalfRad = FMath::DegreesToRadians(BulletSpread);
	ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

	// Bullet end destination
	TracerEnd = Location + (ShotDirection * MaxRange);

	// Set bTraceComplex to true for precision damage to mesh faces
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	Params.bTraceComplex = true;
	Params.bReturnPhysicalMaterial = true;

	if(DebugWeaponDrawing > 0)
	{
		DrawDebugLine(GetWorld(), Location, TracerEnd, FColor::White, false, 1.f, 0, 1.f);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MagCount: %f"), CurrentMagSize));
	}
		
	return GetWorld()->LineTraceSingleByChannel(Hit, Location, TracerEnd, COLLISION_WEAPON, Params);
}


// Function to get owner pawn controller, made for readabilty
AController* ASWeapon::GetOwnerController() const
{
	APawn* OwnerPawn =Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return nullptr;

	return OwnerPawn->GetController();
}


void ASWeapon::PlayWeaponEffects(FVector TraceEndPoint) 
{
	if(MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp =  UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		
		if(TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEndPoint);
		}
	}

	APlayerController* PC = Cast<APlayerController>(GetOwnerController());
	if(PC)
	{
		PC->ClientStartCameraShake(FireCamShake);
	}
}


void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint) 
{
	// Decide which effect to use depending on surface
	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType)
	{
		case SURFACE_FLESHDEFAULT:
		case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;

		default:
		SelectedEffect = DefaultImpactEffect;
		break;		
	}
	// If effect exists spawn at tracer end
	if(SelectedEffect)		
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());			
	}
}


void ASWeapon::OnRep_HitScanTrace() 
{
	PlayWeaponEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}


void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner); // Replicates the Variable to every machine
  DOREPLIFETIME(ASWeapon, BaseMagazineSize);
  DOREPLIFETIME(ASWeapon, CurrentMagSize);
}