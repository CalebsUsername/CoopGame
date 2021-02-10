// Fill out your copyright notice in the Description page of Project Settings.


#include "LaunchPad.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "SCharacter.h"

// Sets default values
ALaunchPad::ALaunchPad()
{
	LaunchComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Launch Component"));
	RootComponent = LaunchComp;

	MeshBaseComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Base Component"));
	MeshBaseComp->SetupAttachment(LaunchComp);

	// Bind to event
	LaunchComp->OnComponentBeginOverlap.AddDynamic(this, &ALaunchPad::HandleOverlap);

	LaunchStrength = 1500.f;
	LaunchPitchAngle = 45.f;

}

void ALaunchPad::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
					   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{

	// Make rotator with our specified 'pitch' and convert to a direction vector * intensity
	FRotator LaunchDirection = GetActorRotation();
	LaunchDirection.Pitch += LaunchPitchAngle;
	FVector LaunchVelocity = LaunchDirection.Vector() * LaunchStrength;

	ASCharacter* OtherCharacter = Cast<ASCharacter>(OtherActor);
	if (OtherCharacter)
	{
		// Launch Player! Both booleans give consistent launch velocity by ignoring the current player velocity
		OtherCharacter->LaunchCharacter(LaunchVelocity, true, true);

		// Spawn FX
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LaunchPadFx, GetActorLocation());
	}
	// Did not overlap a player, so check if it's a physics simulating actor we can launch
	else if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulse(LaunchVelocity, NAME_None, true);

		// Spawn FX
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LaunchPadFx, GetActorLocation());
	}
}
