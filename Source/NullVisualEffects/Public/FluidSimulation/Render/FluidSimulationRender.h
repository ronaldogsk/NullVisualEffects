// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RHIResources.h"
#include "FluidSimulationRender.generated.h"

struct FFluidSimulationVertex
{
public:

    /** Vertex coords */
    FVector2D Coords;

    /** Velocity */
    FVector2D Velocity;

    /** Density */
    float Density;

    /** Constructor */
    FFluidSimulationVertex()
        : Coords(FVector2D::ZeroVector)
        , Velocity(FVector2D::ZeroVector)
        , Density(0.0f)
    {}

    /** Constructor */
    FFluidSimulationVertex
    (
        const FVector2D& InCoords,
        const FVector2D& InVelocity,
        const float InDensity
    )
        : Coords(InCoords)
        , Velocity(InVelocity)
        , Density(InDensity)
    {}
};

struct FFluidCellInputData
{
public:

    /** Add velocity */
    FVector2D Velocity;

    /** Add density */
    float Density;

    /** Add density */
    float Radius;

    /** Add cell */
    FIntPoint Cell;

    /** Constructor */
    FFluidCellInputData()
        : Velocity(FVector2D::ZeroVector)
        , Density(0.0f)
        , Radius(0.0f)
        , Cell(FIntPoint::ZeroValue)
    {}

    /** Constructor */
    FFluidCellInputData(const FVector2D& InVelocity, const float InDensity, const float InRadius, const FIntPoint& InCell)
        : Velocity(InVelocity)
        , Density(InDensity)
        , Radius(InRadius)
        , Cell(InCell)
    {}
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

    /** Enqueues data to be added to  */
    void AddVelocityDensity(const FVector& InLocation, const FVector& InVelocity, const float InRadius, const float InViscosity);

private:

    /** Updates the fluid */
    void UpdateFluid(const float InDeltaTime);

    /** Add input data */
    void AddInputData();

private:

    /** Add input forces and density render thread implementation */
    static void AddInputData_RenderThread(const int32 InSimulationGridSize, const TArray<FFluidCellInputData>& InForcesDensityData, const FUnorderedAccessViewRHIRef& InBufferUAV, FRHICommandListImmediate& RHICmdList);

    /** Update fluid render thread implementation */
    static void UpdateFluid_RenderThread(const int32 InSimulationGridSize, const float InFluidDifusion, const float InFluidViscosity, const float InDeltaTime, const FUnorderedAccessViewRHIRef& InCurrentUAV, const FUnorderedAccessViewRHIRef& InPreviousUAV, FRHICommandListImmediate& RHICmdList);

    /** Draw to render target render thread implementation */
    static void DrawToRenderTarget_RenderThread(class UTextureRenderTarget2D* InRenderTarget, const int32 InSimulationGridSize, const FVertexBufferRHIRef& InVertexBuffer, const FUnorderedAccessViewRHIRef& InBufferUAV, FRHICommandListImmediate& RHICmdList);

protected:

    /** Is the render init */
    bool bIsInit;

    /** Fluid difusion */
    float FluidDifusion;

    /** Fluid viscosity */
    float FluidViscosity;

    /** Output render target */
    class UTextureRenderTarget2D* OutputRenderTarget;

private:

    /** Pending fluid input data to add */
    TArray<FFluidCellInputData> PendingFluidInput;

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