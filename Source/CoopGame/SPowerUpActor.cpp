// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"

// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
	PowerUpInterval = 0.f;
	TotalNumOfTicks = 0;
}


// Called when the game starts or when spawned
void ASPowerUpActor::BeginPlay()
{
	Super::BeginPlay();
	

}


void ASPowerUpActor::OnTickPowerUp() 
{
	TicksProcessed++;

	OnPowerUpTicked();

	if (TicksProcessed >= TotalNumOfTicks)
	{
		OnExpired();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TH_PowerUpTick);
	}
}


void ASPowerUpActor::ActivatePowerUp() 
{
	OnActivated();

	if (PowerUpInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TH_PowerUpTick, this, &ASPowerUpActor::OnTickPowerUp, PowerUpInterval, true);
	}

	else
	{
		OnTickPowerUp();
	}
}
