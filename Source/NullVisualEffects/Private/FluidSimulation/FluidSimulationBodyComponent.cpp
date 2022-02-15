// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "FluidSimulation\FluidSimulationActor.h"
#include "FluidSimulation\FluidSimulationBodyComponent.h"

UFluidSimulationBodyComponent::UFluidSimulationBodyComponent()
    : CurrentLocation(FVector::ZeroVector)
    , PreviousLocation(FVector::ZeroVector)
    , SecondPreviousLocation(FVector::ZeroVector)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = 0.0333f;
}

UFluidSimulationBodyComponent::~UFluidSimulationBodyComponent()
{
}

void UFluidSimulationBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    SecondPreviousLocation = PreviousLocation;
    PreviousLocation = CurrentLocation;
    CurrentLocation = GetComponentLocation();
}

void UFluidSimulationBodyComponent::RegisterComponent(class AFluidSimulationActor* InSimulationActor)
{
    if (InSimulationActor != nullptr)
    {

    }
}