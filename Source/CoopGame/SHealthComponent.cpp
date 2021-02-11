// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SHordeGameMode.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.f;

	SetIsReplicatedByDefault(true);
	bIsDead = false;

	TeamNum = 255;
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = DefaultHealth;

	if(GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}
}


void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) 
{
	if (Damage <= 0.f || bIsDead)
	{
		return;
	}
	
	if (DamageCauser != DamagedActor && IsFriendly(GetOwner(), DamageCauser))
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	bIsDead = Health <= 0.f;

	if (bIsDead)
	{
		ASHordeGameMode* GM = Cast<ASHordeGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
	

}


void USHealthComponent::OnRep_Health(float OldHealth) 
{
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}


void USHealthComponent::Heal(float Amount) 
{
	if (Amount <= 0 || Health <= 0)
	{
		return;
	}
	
	Health = FMath::Clamp(Health + Amount, 0.f, DefaultHealth);
	OnHealthChanged.Broadcast(this, Health, -Amount, nullptr, nullptr, nullptr);
}


float USHealthComponent::GetHealth() const
{
	return Health;
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB) 
{
	if (ActorA == nullptr || ActorB == nullptr) 
	{
		// Assume Friendly
		return true;
	}

	USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		// Assume Friendly
		return true;
	}

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
	
}


void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(USHealthComponent, Health); // Replicates the Variable to every machine
}

