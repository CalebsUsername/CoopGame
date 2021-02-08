// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"
#include "Net/UnrealNetwork.h"



// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
	PowerUpInterval = 0.f;
	TotalNumOfTicks = 0;

	bIsPowerUpActive = false;

	SetReplicates(true);
}


void ASPowerUpActor::OnTickPowerUp() 
{
	TicksProcessed++;

	OnPowerUpTicked();

	if (TicksProcessed >= TotalNumOfTicks)
	{
		OnExpired();

		bIsPowerUpActive = false;
		OnRep_PowerUpActive();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TH_PowerUpTick);
	}
}


void ASPowerUpActor::ActivatePowerUp(AActor* ActivateFor) 
{
	OnActivated(ActivateFor);

	bIsPowerUpActive = true;
	OnRep_PowerUpActive();

	if (PowerUpInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TH_PowerUpTick, this, &ASPowerUpActor::OnTickPowerUp, PowerUpInterval, true);
	}

	else
	{
		OnTickPowerUp();
	}
}


void ASPowerUpActor::OnRep_PowerUpActive() 
{
	OnPowerUpStateChange(bIsPowerUpActive);
}


void ASPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(ASPowerUpActor, bIsPowerUpActive); // Replicates the Variable to every machine
}