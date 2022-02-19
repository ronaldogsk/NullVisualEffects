// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FluidSimulationActor.generated.h"

UCLASS(BlueprintType)
class NULLVISUALEFFECTS_API AFluidSimulationActor : public AActor
{
    GENERATED_BODY()

public:

    /** Constructor */
    AFluidSimulationActor();

    /** Destructor */
    ~AFluidSimulationActor();

public:

    //~ Begin AActor interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
    //~ End AActor interface

public:

    /** Init stuff :) */
    UFUNCTION(CallInEditor, Category = "FluidSimulation")
    void InitResources();

    /** Registers body */
    void RegisterBody(const FVector& InCurrentLocation, const FVector& InPreviousLocation, const FVector& InVelocity, const float InRadius, const float InStrength);

    /** */
    UFUNCTION(CallInEditor, Category = "FluidSimulation")
    void Draw();

public:

    /**  */
    UPROPERTY(EditAnywhere, Category = "FluidSimulation|Simulation")
    int32 SimulationGridSize;

    /**  */
    UPROPERTY(EditAnywhere, Category = "FluidSimulation|Render")
    int32 RenderTargetSize;

    /**  */
    UPROPERTY(EditAnywhere, Category = "FluidSimulation|Material")
    class UStaticMesh* DefaultStaticMesh;

    /**  */
    UPROPERTY(EditAnywhere, Category = "FluidSimulation|Material")
    FName MaterialSlotName;

    /**  */
    UPROPERTY(EditAnywhere, Category = "FluidSimulation|Material")
    FName RenderTargetMaterialParameterName;

    /**  */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* StaticMeshComponent;

private:

    /** Fluid simulation render target */
    UPROPERTY(Transient, VisibleAnywhere)
    class UTextureRenderTarget2D* FluidRenderTarget;

    /**  */
    UPROPERTY(Transient)
    class UMaterialInstanceDynamic* MaterialInstanceDynamic;

    /**  */
    UPROPERTY(Transient)
    class UFluidSimulationRender* FluidSimulationRender;
};
