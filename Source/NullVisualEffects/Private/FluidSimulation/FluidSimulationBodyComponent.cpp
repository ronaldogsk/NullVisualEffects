// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "FluidSimulation\FluidSimulationBodyComponent.h"
#include "FluidSimulation\FluidSimulationActor.h"

UFluidSimulationBodyComponent::UFluidSimulationBodyComponent()
    : MinimumUpdateDistance(5.0f)
    , CurrentLocation(FVector::ZeroVector)
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

void UFluidSimulationBodyComponent::BeginPlay()
{
    Super::BeginPlay();

    OnComponentBeginOverlap.AddDynamic(this, &UFluidSimulationBodyComponent::ComponentBeginOverlap);
    OnComponentEndOverlap.AddDynamic(this, &UFluidSimulationBodyComponent::ComponentEndOverlap);

}

void UFluidSimulationBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentUpdateActor != nullptr)
    {
        CurrentLocation = GetComponentLocation();

        if ((CurrentLocation - PreviousLocation).Size2D() > MinimumUpdateDistance) // #TODO : Add a Recip here
        {
            if (AActor* const Owner = GetOwner())
            {
                CurrentUpdateActor->RegisterBody(CurrentLocation, PreviousLocation, Owner->GetVelocity(), GetScaledSphereRadius(), Strength);
            }

            SecondPreviousLocation = PreviousLocation;
            PreviousLocation = CurrentLocation;
        }
    }
}

void UFluidSimulationBodyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    OnComponentBeginOverlap.RemoveDynamic(this, &UFluidSimulationBodyComponent::ComponentBeginOverlap);
    OnComponentEndOverlap.RemoveDynamic(this, &UFluidSimulationBodyComponent::ComponentEndOverlap);
}

void UFluidSimulationBodyComponent::ComponentBeginOverlap(class UPrimitiveComponent* InOverlappedComp, class AActor* InOtherActor, class UPrimitiveComponent* InOtherComp, int32 InOtherBodyIndex, bool InFromSweep, const FHitResult& InSweepResult)
{
    if (InOtherActor != nullptr)
    {
        CurrentUpdateActor = Cast<AFluidSimulationActor>(InOtherActor);
        CurrentLocation = GetComponentLocation();
        PreviousLocation = CurrentLocation;
        SecondPreviousLocation = CurrentLocation;
    }

}

void UFluidSimulationBodyComponent::ComponentEndOverlap(class UPrimitiveComponent* InOverlappedComp, class AActor* InOtherActor, class UPrimitiveComponent* InOtherComp, int32 InOtherBodyIndex)
{
    CurrentUpdateActor = nullptr;
}
