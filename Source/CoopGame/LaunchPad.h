// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaunchPad.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class COOPGAME_API ALaunchPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaunchPad();

protected:


	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshBaseComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* LaunchComp;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* LaunchPadFx;

	/* Total impulse added to the character on overlap
		Marked 'EditInstanceOnly' to allow in-level editing of this property per instance. */
	UPROPERTY(EditInstanceOnly, Category = "LaunchPad")
	float LaunchStrength;

	/* Angle added on top of actor rotation to launch the character. 
		Marked 'EditInstanceOnly' to allow in-level editing of this property per instance. */ 
	UPROPERTY(EditInstanceOnly, Category = "LaunchPad")
	float LaunchPitchAngle;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,
					   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
