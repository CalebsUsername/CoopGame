// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class UParticleSystem;
class USHealthComponent;
class URadialForceComponent;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplodeFX;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class USoundCue* ExplodeSound;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly)
	USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly)
	URadialForceComponent* BlastComp;

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
	void OnRep_Exploded();

	void PlayExplosionEffects();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};
