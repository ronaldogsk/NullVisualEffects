// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "Components/SphereComponent.h"
#include "CoreMinimal.h"
#include "FluidSimulationBodyComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NULLVISUALEFFECTS_API UFluidSimulationBodyComponent : public USphereComponent
{
    GENERATED_BODY()

public:

    /** Constructor */
    UFluidSimulationBodyComponent();

    /** Destructor */
    ~UFluidSimulationBodyComponent();

    //~ Begin USceneComponent interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End USceneComponent interface

private:

    UFUNCTION()
    void ComponentBeginOverlap(class UPrimitiveComponent* InOverlappedComp, class AActor* InOtherActor, class UPrimitiveComponent* InOtherComp, int32 InOtherBodyIndex, bool InFromSweep, const FHitResult& InSweepResult);

    UFUNCTION()
    void ComponentEndOverlap(class UPrimitiveComponent* InOverlappedComp, class AActor* InOtherActor, class UPrimitiveComponent* InOtherComp, int32 InOtherBodyIndex);

public:

    UPROPERTY(EditAnywhere, Category = "FluidSimulation")
    float MinimumUpdateDistance;

    UPROPERTY(EditAnywhere, Category = "FluidSimulation")
    float Strength;

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

    /**  */
    class AFluidSimulationActor* CurrentUpdateActor;

};