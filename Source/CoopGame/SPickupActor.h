// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class ASPowerUpActor;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickupActor();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UDecalComponent* DecalComp;

	UPROPERTY(EditAnywhere, Category = "PickupActor")
	TSubclassOf<ASPowerUpActor> PowerUpClass;

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
	float CooldownDuration;

	FTimerHandle TH_RespawnTimer;

	void Respawn();

	ASPowerUpActor* PowerUpInstance;
};
