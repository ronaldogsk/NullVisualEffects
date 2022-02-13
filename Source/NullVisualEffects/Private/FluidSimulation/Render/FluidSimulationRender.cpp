// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "FluidSimulation/Render/FluidSimulationRender.h"
#include "FluidSimulation/Render/FluidSimulationCS.h"
#include "FluidSimulation/Render/FluidSimulationDrawCS.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"

const FVertexDeclarationElementList UFluidSimulationRender::VertexSimulationDataDeclaration
{
    FVertexElement(0, offsetof(FFluidSimulationVertex, Velocity), EVertexElementType::VET_Float3, 0, sizeof(FFluidSimulationVertex))
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
    SimulationGridSize = InSimulationGridSize;
    bIsInit = SimulationGridSize > 0;

    if (bIsInit)
    {
        ENQUEUE_RENDER_COMMAND(CreateBuffer)
        (
            [&](FRHICommandListImmediate& RHICmdList)
            {
                TResourceArray<FFluidSimulationVertex, VERTEXBUFFER_ALIGNMENT> VertexBufferData;

                for (int32 i = 0; i < InSimulationGridSize; ++i)
                {
                    for (int32 j = 0; j < InSimulationGridSize; ++j)
                    {
                        VertexBufferData.Add(FFluidSimulationVertex());
                    }
                }

                FRHIResourceCreateInfo ResourceCreateInfo(&VertexBufferData);
                const uint32 BufferUsage = BUF_Static | BUF_UnorderedAccess | BUF_ShaderResource;

                VertexBuffer = RHICreateVertexBuffer(VertexBufferData.GetResourceDataSize(), BufferUsage, ResourceCreateInfo);
                VertexBufferUAV = RHICreateUnorderedAccessView(VertexBuffer.GetReference(), PF_R32_FLOAT);
            }
        );
    }

    FlushRenderingCommands();

    return bIsInit;
}

void UFluidSimulationRender::UpdateFluid(const float InDeltaTime)
{
    ENQUEUE_RENDER_COMMAND(FluidSimulationRender_UpdateFluid)
    (
        [
            DeltaTime           = InDeltaTime,
            UAV                 = VertexBufferUAV,
            SimulationGridSize  = SimulationGridSize
        ]
        (FRHICommandListImmediate& RHICmdList)
        {
            UpdateFluid_RenderThread(DeltaTime, SimulationGridSize, UAV, RHICmdList);
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
                FluidBufferUAV = VertexBufferUAV
            ]
            (FRHICommandListImmediate& RHICmdList)
            {
                DrawToRenderTarget_RenderThread(RenderTarget, FluidBufferUAV, RHICmdList);
            }
        );
    }
}

void UFluidSimulationRender::UpdateFluid_RenderThread(const float InDeltaTime, const int32 InSimulationGridSize, const FUnorderedAccessViewRHIRef& InUAV, FRHICommandListImmediate& RHICmdList)
{
    check(IsInRenderingThread());
    QUICK_SCOPE_CYCLE_COUNTER(STAT_FluidSimulationRender_UpdateFluid_RenderThread);
    SCOPED_DRAW_EVENT(RHICmdList, FluidSimulationRender_UpdateFluid_RenderThread);

    FFluidSimulationCS::FParameters Params;
    Params.FluidData = InUAV;
    Params.DeltaTime = InDeltaTime;

    TShaderMapRef<FFluidSimulationCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    FIntVector GroupCount = FIntVector(InSimulationGridSize, InSimulationGridSize, 1); // Temporary usage of a Raw group count 'InSimulationGridSize * InSimulationGridSize'
    FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, Params, GroupCount);
}

void UFluidSimulationRender::DrawToRenderTarget_RenderThread(class UTextureRenderTarget2D* InRenderTarget, const FUnorderedAccessViewRHIRef& InBufferUAV, FRHICommandListImmediate& RHICmdList)
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

        TShaderMapRef<FFluidSimulationDrawCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        FIntVector GroupCount = FIntVector(InRenderTarget->SizeX, InRenderTarget->SizeY, 1);
        FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, Params, GroupCount);

        RHICmdList.CopyToResolveTarget(ComputeShaderOutput.GetReference()->GetRenderTargetItem().ShaderResourceTexture, InRenderTarget->GetRenderTargetResource()->TextureRHI->GetTexture2D(), FResolveParams());
    }
}