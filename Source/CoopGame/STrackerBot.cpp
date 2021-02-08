// Fill out your copyright notice in the Description page of Project Settings.


#include "STrackerBot.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SHealthComponent.h"
#include "SCharacter.h"
#include "Sound/SoundCue.h"
#include "EngineUtils.h"



static int32 DebugBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugBotDrawing
(
	TEXT("COOP.DebugBots"), 
	DebugBotDrawing, 
	TEXT("Draw Debug Lines For Bots"),
	ECVF_Cheat
);


// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	// Health component made for custom health events
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	// overlap sphere to intiate self destruct and call self damage
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	// variables used for AI movement
	bUseVelocityChange = true;
	MovementForce = 1000.f;
	RequiredDistanceToTarget = 100.f;

	ExplosionDamage = 100.f;
	ExplosionRadius = 250.f;
	SelfDamageInterval = .3f;
}


// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		// Find intitial MoveTo
		NextPathPoint = GetNextPathPoint();

		// Every second we update our power-level based on nearby bots (CHALLENGE CODE)
		FTimerHandle TH_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TH_CheckPowerLevel, this, &ASTrackerBot::OnCheckForFriendlyBots, 1.0f, true);
	}
}


// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(HasAuthority() && !bExploded)
	{
		// Path find to a players location
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if(DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
		}

		else
		{
			// Keep moving towards next target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
		}
	}
}


FVector ASTrackerBot::GetNextPathPoint() 
{

	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);					
	
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	GetWorldTimerManager().ClearTimer(TH_RefreshPath);
	GetWorldTimerManager().SetTimer(TH_RefreshPath, this, &ASTrackerBot::RefreshPath , 5.0f, false);

	if(NavPath && NavPath->PathPoints.Num() > 1)
	{
		// Return next point in path
		return NavPath->PathPoints[1];
	}

	// Failed to find path
	return GetActorLocation();
}


void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}


// Override function to start the self destruct sequence
// on overlapping with a player
void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor) 
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	if(!bSelfDestructInitiated)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

		// Check to see if overlapped with a player
		if (PlayerPawn)
		{
			if(HasAuthority())
			{
				// Start self destruction sequence
				GetWorldTimerManager().SetTimer(TH_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.f);
			}

			bSelfDestructInitiated = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}		
	}
}


// Used in the Health component to determine health as assigned in constructor
void ASTrackerBot::HandleTakeDamage(USHealthComponent* HealthComponent, float Health, float HealthDelta, 
								  const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) 
{
	// Assign material instance dynamically 
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	// If material instance exists set parameter to make material 'Flash' on taking any damage
	if(MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	if(Health <= 0)
	{
		SelfDestruct();
	}
		
	UE_LOG(LogTemp, Warning, TEXT("Health %s of %s: "), *FString::SanitizeFloat(Health), *GetName());
}


void ASTrackerBot::SelfDestruct() 
{
	if(bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), FRotator::ZeroRotator, ((FVector)((2.0F))), true, EPSCPoolMethod::None, true);
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	MeshComp->SetVisibility(false, true);

	if(HasAuthority())
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);

		float ActualDamage = ExplosionDamage += (ExplosionDamage * PowerLevel);

		// Apply damage
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoreActors, this, GetInstigatorController(), true);

		// Debug
		if (DebugBotDrawing > 0)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Blue, false, 4.f, (uint8)'\000', 3.f);
		}

		SetLifeSpan(1.f);
	}
}


// Damaging self to lead to self destruction
void ASTrackerBot::DamageSelf() 
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}


void ASTrackerBot::OnCheckForFriendlyBots() 
{
	// distance to check for nearby bots
	const float Radius = 200.f;

	// Create temporary collision shape for overlaps
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);


	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);	// Our tracker bot's mesh component is set to Physics Body in Blueprint
	QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic); // used to find ASTracker objects

	TArray<FOverlapResult> Overlaps;
	TArray<ASTrackerBot*> BotsFound;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	int32 NrOfBots = 0;

	for (FOverlapResult Result : Overlaps)
	{
		// Check if we overlapped with another tracker bot (ignoring players and other bot types)
		BotsFound.AddUnique(Cast<ASTrackerBot>(Result.GetActor()));
		// Ignore this trackerbot instance
		if (BotsFound.Num() > 0)
		{
			NrOfBots = BotsFound.Num();
		}
	}

	const int32 MaxPowerLevel = 4;

	// Clamp between min=0 and max=4
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

	// Debug
	if (DebugBotDrawing > 0)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Blue, false, 4.f, (uint8)'\000', 3.f);
	}

	// Update the material color
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		// Convert to a float between 0 and 1 just like an 'Alpha' value of a texture. Now the material can be set up without having to know the max power level 
		// which can be tweaked many times by gameplay decisions (would mean we need to keep 2 places up to date)
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		// Note: (float)MaxPowerLevel converts the int32 to a float, 
		//	otherwise the following happens when dealing when dividing integers: 1 / 4 = 0 ('PowerLevel' int / 'MaxPowerLevel' int = 0 int)
		//	this is a common programming problem and can be fixed by 'casting' the int (MaxPowerLevel) to a float before dividing.

		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}
}