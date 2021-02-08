// Fill out your copyright notice in the Description page of Project Settings.


#include "SHordeGameMode.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "SHealthComponent.h"
#include "EngineUtils.h"



ASHordeGameMode::ASHordeGameMode() 
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.f;

    TimeBetweenWaves = 2.f;
}


void ASHordeGameMode::Tick(float DeltaTime) 
{
    Super::Tick(DeltaTime);

    CheckWaveState();
}


void ASHordeGameMode::StartWave() 
{
    WaveCount++;
    NumOfBotsToSpawn = 2 * WaveCount;

    GetWorldTimerManager().SetTimer(TH_BotSpawner, this, &ASHordeGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);
}


void ASHordeGameMode::EndWave() 
{
    GetWorldTimerManager().ClearTimer(TH_BotSpawner);
}


void ASHordeGameMode::PrepareForNextWave() 
{
    GetWorldTimerManager().SetTimer(TH_NextWaveStart, this, &ASHordeGameMode::StartWave, TimeBetweenWaves, false);
}


void ASHordeGameMode::CheckWaveState() 
{
    bool bIsPreparingWave = GetWorldTimerManager().IsTimerActive(TH_NextWaveStart);

    if (NumOfBotsToSpawn > 0 || bIsPreparingWave) {return;}

    bool bIsAnyBotAlive = false;

    for (class TActorIterator<APawn> Itr(GetWorld()); Itr; ++Itr)
    {
        APawn* TestPawn = *Itr;

        if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
        {
            continue;
        }
        
        USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
        if (HealthComp != nullptr && HealthComp->GetHealth() > 0.f)
        {
           bIsAnyBotAlive = true;
           break;
        }
    }

    if (!bIsAnyBotAlive)
    {
        PrepareForNextWave();
    }
    
}


void ASHordeGameMode::SpawnBotTimerElapsed() 
{
    SpawnNewBot();

    NumOfBotsToSpawn--;

    if (NumOfBotsToSpawn <= 0)
    {
        EndWave();
    }
}


void ASHordeGameMode::StartPlay() 
{
    Super::StartPlay();

    PrepareForNextWave();
}
