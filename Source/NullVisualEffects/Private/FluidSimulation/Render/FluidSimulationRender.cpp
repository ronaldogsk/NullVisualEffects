// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "FluidSimulation/Render/FluidSimulationRender.h"
#include "FluidSimulation/Render/FluidSimulationCS.h"
#include "FluidSimulation/Render/FluidSimulationDrawCS.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Library/NullVisualEffectsFunctionLibrary.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"

const FVertexDeclarationElementList UFluidSimulationRender::VertexSimulationDataDeclaration
{
    FVertexElement(0, offsetof(FFluidSimulationVertex, Velocity)    , EVertexElementType::VET_Float2, 0, sizeof(FFluidSimulationVertex)),
    FVertexElement(0, offsetof(FFluidSimulationVertex, Density)     , EVertexElementType::VET_Float1, 1, sizeof(FFluidSimulationVertex))
};

UFluidSimulationRender::UFluidSimulationRender()
    : bIsInit(false)
    , SimulationGridSize(0)
{
}

UFluidSimulationRender::~UFluidSimulationRender()
{
}

void UFluidSimulationRender::BeginDestroy()
{
    Super::BeginDestroy();

    RenderFence.Wait();
}

bool UFluidSimulationRender::IsReadyForFinishDestroy()
{
    return Super::IsReadyForFinishDestroy() && RenderFence.IsFenceComplete();
}

void UFluidSimulationRender::Tick(float DeltaTime)
{
    RenderFence.BeginFence(true);

    if (bIsInit)
    {
        UpdateFluid(DeltaTime);

        if (OutputRenderTarget != nullptr)
        {
            DrawToRenderTarget(OutputRenderTarget);
        }
    }
}

bool UFluidSimulationRender::IsTickable() const
{
    return true;
}

bool UFluidSimulationRender::IsTickableInEditor() const
{
    return true;
}

bool UFluidSimulationRender::IsTickableWhenPaused() const
{
    return false;
}

TStatId UFluidSimulationRender::GetStatId() const
{
    return TStatId();
}

bool UFluidSimulationRender::Init(const int32 InSimulationGridSize)
{
    RenderFence.Wait();

    VertexBuffer.SafeRelease();
    VertexBufferUAV.SafeRelease();
    SpareVertexBuffer.SafeRelease();
    SpareVertexBufferUAV.SafeRelease();

    SimulationGridSize = InSimulationGridSize;
    bIsInit = SimulationGridSize > 0;

    if (bIsInit)
    {
        ENQUEUE_RENDER_COMMAND(CreateBuffer)
        (
            [ this ]
            (FRHICommandListImmediate& RHICmdList)
            {
                TResourceArray<FFluidSimulationVertex, VERTEXBUFFER_ALIGNMENT> VertexBufferData;

                const float SimulationGridRecip = 1.0f / SimulationGridSize;
                for (int32 i = 0; i < SimulationGridSize; ++i)
                {
                    for (int32 j = 0; j < SimulationGridSize; ++j)
                    {
                        const FVector& RandomVector = FMath::VRand();
                        VertexBufferData.Emplace(FFluidSimulationVertex(FVector2D(RandomVector).GetSafeNormal(), 0.0f));
                    }
                }

                FRHIResourceCreateInfo ResourceCreateInfo(&VertexBufferData);
                const uint32 BufferUsage = BUF_Static | BUF_UnorderedAccess | BUF_ShaderResource;

                VertexBuffer = RHICreateVertexBuffer(VertexBufferData.GetResourceDataSize(), BufferUsage, ResourceCreateInfo);
                VertexBufferUAV = RHICreateUnorderedAccessView(VertexBuffer.GetReference(), PF_R32_FLOAT);

                SpareVertexBuffer = RHICreateVertexBuffer(VertexBufferData.GetResourceDataSize(), BufferUsage, ResourceCreateInfo);
                SpareVertexBufferUAV = RHICreateUnorderedAccessView(SpareVertexBuffer.GetReference(), PF_R32_FLOAT);
            }
        );

        FlushRenderingCommands();
    }

    return bIsInit;
}

void UFluidSimulationRender::UpdateFluid(const float InDeltaTime)
{
    ENQUEUE_RENDER_COMMAND(FluidSimulationRender_UpdateFluid)
    (
        [
            DeltaTime           = InDeltaTime,
            CurrentUAV          = VertexBufferUAV,
            PreviousUAV         = SpareVertexBufferUAV,
            SimulationGridSize  = SimulationGridSize,
            FluidDifusion       = FluidDifusion,
            FluidViscosity      = FluidViscosity

        ]
        (FRHICommandListImmediate& RHICmdList)
        {
            UpdateFluid_RenderThread(SimulationGridSize, FluidDifusion, FluidViscosity, DeltaTime, CurrentUAV, PreviousUAV, RHICmdList);
        }
    );
}

void UFluidSimulationRender::DrawToRenderTarget(class UTextureRenderTarget2D* InRenderTarget)
{
    if (InRenderTarget != nullptr && InRenderTarget->SizeX == SimulationGridSize && InRenderTarget->SizeY == SimulationGridSize)
    {
        ENQUEUE_RENDER_COMMAND(FluidSimulationRender_DrawToRenderTarget)
        (
            [
                RenderTarget = InRenderTarget,
                FluidBufferUAV = VertexBufferUAV,
                SimulationGridSize = SimulationGridSize
            ]
            (FRHICommandListImmediate& RHICmdList)
            {
                DrawToRenderTarget_RenderThread(RenderTarget, SimulationGridSize, FluidBufferUAV, RHICmdList);
            }
        );
    }
}

void UFluidSimulationRender::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
    OutputRenderTarget = InRenderTarget;
}

void UFluidSimulationRender::UpdateFluid_RenderThread(const int32 InSimulationGridSize, const float InFluidDifusion, const float InFluidViscosity, const float InDeltaTime, const FUnorderedAccessViewRHIRef& InCurrentUAV, const FUnorderedAccessViewRHIRef& InPreviousUAV, FRHICommandListImmediate& RHICmdList)
{
    check(IsInRenderingThread());
    QUICK_SCOPE_CYCLE_COUNTER(STAT_FluidSimulationRender_UpdateFluid_RenderThread);
    SCOPED_DRAW_EVENT(RHICmdList, FluidSimulationRender_UpdateFluid_RenderThread);

    FFluidSimulationCS::FParameters Params;
    Params.CurrentFluidData = InCurrentUAV;
    Params.PreviousFluidData = InPreviousUAV;
    Params.SimulationGridSize = InSimulationGridSize;
    Params.SimulationGridSizeRecip = 1.0f / static_cast<float>(InSimulationGridSize);
    Params.FluidDifusion = InFluidDifusion;
    Params.FluidViscosity = InFluidViscosity;
    Params.DeltaTime = InDeltaTime;

    TShaderMapRef<FFluidSimulationCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    FIntVector GroupCount = FIntVector(InSimulationGridSize * InSimulationGridSize, 1, 1); // Temporary usage of a Raw group count 'InSimulationGridSize * InSimulationGridSize'
    FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, Params, GroupCount);
}

void UFluidSimulationRender::DrawToRenderTarget_RenderThread(class UTextureRenderTarget2D* InRenderTarget, const int32 InSimulationGridSize, const FUnorderedAccessViewRHIRef& InBufferUAV, FRHICommandListImmediate& RHICmdList)
{
    check(IsInRenderingThread());
    QUICK_SCOPE_CYCLE_COUNTER(STAT_FluidSimulationRender_DrawToRenderTarget_RenderThread);
    SCOPED_DRAW_EVENT(RHICmdList, FluidSimulationRender_UDrawToRenderTarget_RenderThread);

    if (InRenderTarget != nullptr)
    {
        // Gets a render target from RenderTargetPool
        TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;
        FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(InRenderTarget->SizeX, InRenderTarget->SizeY), InRenderTarget->GetFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
        ComputeShaderOutputDesc.DebugName = TEXT("DrawToRenderTarget");
        GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, ComputeShaderOutput, TEXT("DrawToRenderTarget"));

        FFluidSimulationDrawCS::FParameters Params;
        Params.OutTexture = ComputeShaderOutput.GetReference()->GetRenderTargetItem().UAV;
        Params.FluidData = InBufferUAV;
        Params.SimulationGridSize = InSimulationGridSize;
        Params.SimulationGridSizeRecip = 1.0f / static_cast<float>(InSimulationGridSize);

        TShaderMapRef<FFluidSimulationDrawCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        FIntVector GroupCount = FIntVector(InSimulationGridSize * InSimulationGridSize, 1, 1);
        FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, Params, GroupCount);

        RHICmdList.CopyToResolveTarget(ComputeShaderOutput.GetReference()->GetRenderTargetItem().ShaderResourceTexture, InRenderTarget->GetRenderTargetResource()->TextureRHI->GetTexture2D(), FResolveParams());
    }
}