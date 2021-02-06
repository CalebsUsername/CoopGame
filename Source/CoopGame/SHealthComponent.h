// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams
(
FOnHealthComponentChangedSignature, 
USHealthComponent*, HealthComp, 
float, Health, 
float, HealthDelta, 
const class UDamageType*, DamageType, 
class AController*, InstigatedBy, 
AActor*, DamageCauser
);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthComponentChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Health Component")
	void Heal(float Amount);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;
};
