// Fill out your copyright notice in the Description page of Project Settings.


#include "AGameState.h"
#include "Net/UnrealNetwork.h"


void AAGameState::OnRep_WaveState(EWaveState OldState) 
{
    WaveStateChanged(WaveState, OldState);
}


void AAGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(AAGameState, WaveState); // Replicates the Variable to every machine
}


void AAGameState::SetWaveState(EWaveState NewState) 
{
    if (HasAuthority())
    {
        EWaveState OldState = NewState;

        WaveState = NewState;
        // Call on server
        OnRep_WaveState(OldState);
    }
}
