// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SHordeGameMode.generated.h"


enum class EWaveState : uint8;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);


UCLASS()
class COOPGAME_API ASHordeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ASHordeGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;

protected:

	FTimerHandle TH_BotSpawner;

	FTimerHandle TH_NextWaveStart;

	// Bots to spawn in current wave
	int32 NumOfBotsToSpawn;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	// Hook for BP to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	// Start Spawning bots
	void StartWave();

	// Stop spawning bots
	void EndWave();

	// Start timer for next start wave
	void PrepareForNextWave();

	void CheckWaveState();
	
	void CheckForPlayersLiving();

	void GameOver();

	void SetWaveState(EWaveState NewState);
};
