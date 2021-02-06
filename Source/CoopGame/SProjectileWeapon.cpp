// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"



ASProjectileWeapon::ASProjectileWeapon() 
{
   
}

void ASProjectileWeapon::Fire() 
{

    AController* OwnerController = GetOwnerController();
	if(OwnerController == nullptr) {return;}

	FVector Location;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Location, Rotation);


    if (ProjectileActor)
    {
        FVector MuzzleLocation = MeshComp->GetSocketLocation("MuzzleSocket");
		FRotator MuzzleRotation = MeshComp->GetSocketRotation("MuzzleSocket");

        FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        GetWorld()->SpawnActor<AActor>(ProjectileActor, MuzzleLocation, Rotation, ActorSpawnParams);
    }
}
