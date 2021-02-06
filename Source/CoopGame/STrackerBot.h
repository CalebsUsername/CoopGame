// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"


UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage
	(
		USHealthComponent* HealthComponent, 
		float Health, 
		float HealthDelta, 
		const class UDamageType* DamageType, 
		class AController* InstigatedBy, 
		AActor* DamageCauser
	);

	FVector GetNextPathPoint();

	void RefreshPath();

	FTimerHandle TH_RefreshPath;

	// Next point in navigation path
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;
	
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	// Dynamic material to pulse on take damage
	UMaterialInstanceDynamic* MatInst;

	void OnCheckForFriendlyBots();

	int32 PowerLevel;

	bool bExploded;

	bool bSelfDestructInitiated;

	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class USoundCue* SelfDestructSound; 

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class USoundCue* ExplodeSound; 

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float SelfDamageInterval;

	FTimerHandle TH_SelfDamage;

	void DamageSelf();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
