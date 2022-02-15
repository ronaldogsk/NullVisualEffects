// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RHIResources.h"
#include "FluidSimulationRender.generated.h"

struct FFluidSimulationVertex
{
public:

    /** Velocity */
    FVector2D Velocity;

    /** Density */
    float Density;

    /** Constructor */
    FFluidSimulationVertex()
        : Velocity(FVector2D::ZeroVector)
        , Density(0.0f)
    {}

    /** Constructor */
    FFluidSimulationVertex
    (
        const FVector2D& InVelocity,
        const float InDensity
    )
        : Velocity(InVelocity)
        , Density(InDensity)
    {}
};

struct FFluidSimulationForce
{
    //FVector
};

UCLASS()
class NULLVISUALEFFECTS_API UFluidSimulationRender : public UObject, public FTickableGameObject
{
    GENERATED_BODY()

public:

    /** Constructor */
    UFluidSimulationRender();

    /** Destructor */
    ~UFluidSimulationRender();

    //~ Begin FTickableGameObject interface
    virtual void BeginDestroy() override;
    virtual bool IsReadyForFinishDestroy() override;
    //~ End FTickableGameObject interface

    //~ Begin FTickableGameObject interface
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual bool IsTickableInEditor() const override;
    virtual bool IsTickableWhenPaused() const override;
    virtual TStatId GetStatId() const override;
    //~ End FTickableGameObject interface

public:

    /** Init render object */
    bool Init(const int32 InSimulationGridSize);

    /** 
     * Draws the current simulation state onto a Render Target,
     * for now I'll be using a compute shader to draw the result
     * so it's easier to visualize results.
     * 
     * #TODO 
     * Further in the future will use Vertex and Pixel shader so
     * it's possible to draw in bigger Render Targets.
     */
    void DrawToRenderTarget(class UTextureRenderTarget2D* InRenderTarget);

    /** Sets the output render target */
    void SetRenderTarget(class UTextureRenderTarget2D* InRenderTarget);

private:

    /** Updates the fluid */
    void UpdateFluid(const float InDeltaTime);

private:

    /** Update fluid render thread implementation */
    static void UpdateFluid_RenderThread(const int32 InSimulationGridSize, const float InFluidDifusion, const float InFluidViscosity, const float InDeltaTime, const FUnorderedAccessViewRHIRef& InCurrentUAV, const FUnorderedAccessViewRHIRef& InPreviousUAV, FRHICommandListImmediate& RHICmdList);

    /** Draw to render target render thread implementation */
    static void DrawToRenderTarget_RenderThread(class UTextureRenderTarget2D* InRenderTarget, const int32 InSimulationGridSize, const FUnorderedAccessViewRHIRef& InBufferUAV, FRHICommandListImmediate& RHICmdList);

protected:

    /** Is the render init */
    bool bIsInit;

    /**  */
    float FluidDifusion;

    /**  */
    float FluidViscosity;

    /** Output render target */
    class UTextureRenderTarget2D* OutputRenderTarget;

private:

    /** Simulation grid size */
    int32 SimulationGridSize;

    /** Vertex buffer */
    FVertexBufferRHIRef VertexBuffer; 

    /** Vertex buffer unordered access view */
    FUnorderedAccessViewRHIRef VertexBufferUAV;

    /** Spare vertex buffer */
    FVertexBufferRHIRef SpareVertexBuffer;

    /** Spare vertex buffer unordered access view */
    FUnorderedAccessViewRHIRef SpareVertexBufferUAV;

    /** Render command fence */
    FRenderCommandFence RenderFence;

public:

    /** Fluid simulation vertex data declaration */
    static const FVertexDeclarationElementList VertexSimulationDataDeclaration;

};