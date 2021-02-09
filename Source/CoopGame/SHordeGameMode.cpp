// Fill out your copyright notice in the Description page of Project Settings.


#include "SHordeGameMode.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "SHealthComponent.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "AGameState.h"
#include "SPlayerState.h"



ASHordeGameMode::ASHordeGameMode() 
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.f;

    PlayerStateClass = ASPlayerState::StaticClass();
    GameStateClass = AAGameState::StaticClass();

    TimeBetweenWaves = 2.f;
}


void ASHordeGameMode::Tick(float DeltaTime) 
{
    Super::Tick(DeltaTime);

    CheckWaveState();
    CheckForPlayersLiving();
}


void ASHordeGameMode::StartPlay() 
{
    Super::StartPlay();

    PrepareForNextWave();
}


void ASHordeGameMode::StartWave() 
{
    WaveCount++;
    NumOfBotsToSpawn = 2 * WaveCount;

    GetWorldTimerManager().SetTimer(TH_BotSpawner, this, &ASHordeGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);

    SetWaveState(EWaveState::WaveInProgress);
}


void ASHordeGameMode::EndWave() 
{
    GetWorldTimerManager().ClearTimer(TH_BotSpawner);
    SetWaveState(EWaveState::WaitingToComplete);
}


void ASHordeGameMode::PrepareForNextWave() 
{
    GetWorldTimerManager().SetTimer(TH_NextWaveStart, this, &ASHordeGameMode::StartWave, TimeBetweenWaves, false);
    SetWaveState(EWaveState::WaitingToStart);
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
        SetWaveState(EWaveState::WaveComplete);
    }
    
}

void ASHordeGameMode::CheckForPlayersLiving() 
{
    for (class TActorIterator<APlayerController> Itr(GetWorld()); Itr; ++Itr)
    {
        APlayerController* PC = *Itr;
        if (PC && PC->GetPawn())
        {
            APawn* MyPawn = PC->GetPawn();
            USHealthComponent * HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
            if (ensure(HealthComp) && HealthComp->GetHealth() > 0.f)
            {
                // At least one player is still alive
                return;
            }
        }
    }
    // No players left alive
    GameOver();
}

void ASHordeGameMode::GameOver() 
{
    EndWave();
    // TODO Finish Match and end the game
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("GAME OVER!!  No players left alive!!")));

    SetWaveState(EWaveState::GameOver);
}

void ASHordeGameMode::SetWaveState(EWaveState NewState) 
{
    AAGameState* GS = GetGameState<AAGameState>(); 
    if (ensureAlways(GS))
    {
        GS->SetWaveState(NewState);
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