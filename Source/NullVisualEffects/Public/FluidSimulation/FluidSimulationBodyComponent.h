// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "FluidSimulationBodyComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NULLVISUALEFFECTS_API UFluidSimulationBodyComponent : public USceneComponent
{
    GENERATED_BODY()

public:

    /** Constructor */
    UFluidSimulationBodyComponent();

    /** Destructor */
    ~UFluidSimulationBodyComponent();

    //~ Begin USceneComponent interface
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //~ End USceneComponent interface

public:

    /** Register body component action */
    void RegisterComponent(class AFluidSimulationActor* InSimulationActor);

private:

    /** Current location */
    FVector CurrentLocation;

    /** Previous location */
    FVector PreviousLocation;

    /**
     * Second previous location.
     * In some cases in the future might be needed for sake of accuracy
     */
    FVector SecondPreviousLocation;

};