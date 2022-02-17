// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "FluidSimulation/Render/FluidSimulationRender.h"
#include "FluidSimulation/Render/FluidSimulationCS.h"
#include "FluidSimulation/Render/FluidSimulationAddInputCS.h"
#include "FluidSimulation/Render/FluidSimulationDrawCS.h"
#include "FluidSimulation/Render/FluidSimulationVS.h"
#include "FluidSimulation/Render/FluidSimulationPS.h"
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
        AddVelocityDensity(FVector::ZeroVector, FVector::OneVector, 0.0f);
        AddInputData();
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

void UFluidSimulationRender::AddInputData()
{
    ENQUEUE_RENDER_COMMAND(FluidSimulationRender_AddInputData)
    (
        [
            SimulationGridSize  = SimulationGridSize,
            InputData           = PendingFluidInput,
            CurrentUAV          = VertexBufferUAV
        ]
        (FRHICommandListImmediate& RHICmdList)
        {
            AddInputData_RenderThread(SimulationGridSize, InputData, CurrentUAV, RHICmdList);
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
                RenderTarget            = InRenderTarget,
                FluidVertexBuffer       = VertexBuffer,
                FluidVertexBufferUAV    = VertexBufferUAV,
                SimulationGridSize      = SimulationGridSize
            ]
            (FRHICommandListImmediate& RHICmdList)
            {
                DrawToRenderTarget_RenderThread(RenderTarget, SimulationGridSize, FluidVertexBuffer, FluidVertexBufferUAV, RHICmdList);
            }
        );
    }
}

void UFluidSimulationRender::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
    OutputRenderTarget = InRenderTarget;
}

void UFluidSimulationRender::AddVelocityDensity(const FVector& InLocation, const FVector& InVelocity, const float InViscosity)
{
    PendingFluidInput.Emplace(FFluidCellInputData(FVector2D(InVelocity), 0.0f, FIntPoint(127, 127))); // Now hardcoded to the middle, to be removed.
}

void UFluidSimulationRender::AddInputData_RenderThread(const int32 InSimulationGridSize, const TArray<FFluidCellInputData>& InForcesDensityData, const FUnorderedAccessViewRHIRef& InBufferUAV, FRHICommandListImmediate& RHICmdList)
{
    check(IsInRenderingThread());
    QUICK_SCOPE_CYCLE_COUNTER(STAT_ForcesDencityData_RenderThread);
    SCOPED_DRAW_EVENT(RHICmdList, ForcesDencityData_RenderThread);

    if (InForcesDensityData.Num() > 0)
    {
        const uint32 BufferUsage = BUF_Static | BUF_UnorderedAccess | BUF_ShaderResource | BUF_StructuredBuffer;

        TResourceArray<FFluidCellInputData> BufferData(false);
        for (const FFluidCellInputData& Element : InForcesDensityData)
        {
            BufferData.Emplace(Element);
        }

        FRHIResourceCreateInfo CreateInfo(&BufferData);
        FStructuredBufferRHIRef StructuredBufferRef = RHICreateStructuredBuffer(sizeof(FFluidCellInputData), sizeof(FFluidCellInputData) * BufferData.Num(), BufferUsage, CreateInfo);
        FUnorderedAccessViewRHIRef BufferUAVRef = RHICreateUnorderedAccessView(StructuredBufferRef, true, false);

        FFluidSimulationAddInputCS::FParameters Params;
        Params.CurrentFluidData = InBufferUAV;
        Params.SimulationGridSize = InSimulationGridSize;
        Params.SimulationGridSizeRecip = 1.0f / static_cast<float>(InSimulationGridSize);
        Params.ForcesDencityData = BufferUAVRef;

        TShaderMapRef<FFluidSimulationAddInputCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        FIntVector GroupCount = FIntVector(InForcesDensityData.Num(), 1, 1);
        FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, Params, GroupCount);
    }
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

void UFluidSimulationRender::DrawToRenderTarget_RenderThread(class UTextureRenderTarget2D* InRenderTarget, const int32 InSimulationGridSize, const FVertexBufferRHIRef& InVertexBuffer, const FUnorderedAccessViewRHIRef& InBufferUAV, FRHICommandListImmediate& RHICmdList)
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

    // if (InRenderTarget != nullptr)
    // {
    //     if (FTextureRenderTargetResource* const DrawResource = InRenderTarget->GetRenderTargetResource())
    //     {
    //         const FTexture2DRHIRef& DrawTarget = DrawResource->GetRenderTargetTexture();
    // 
    //         // Create render pass for render target
    //         FRHIRenderPassInfo RenderPassInfo(DrawTarget, ERenderTargetActions::Clear_Store);
    //         RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("FluidSimulationDrawToRenderTarget"));
    //         {
    //             FIntVector DstSizeXYZ = DrawTarget->GetSizeXYZ();
    //             RHICmdList.SetViewport(0.0f, 0.0f, 0.0f, DstSizeXYZ.X, DstSizeXYZ.Y, 0.0f);
    // 
    //             // Get Shaders from global shader map
    //             auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
    //             TShaderMapRef<FFluidSimulationVS> VertexShader(ShaderMap);
    //             TShaderMapRef<FFluidSimulationPS> PixelShader(ShaderMap);
    // 
    //             // Create PSO
    //             FGraphicsPipelineStateInitializer GraphicsPSOInit;
    //             RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
    // 
    //             GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One>::GetRHI();
    //             GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
    //             GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
    //             GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
    //             GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
    //             GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
    // 
    //             /*
    //                 FIndexBufferRHIRef I'll need this :D
    //                 C:\Program Files (x86)\Epic Games\UE_4.27\Engine\Plugins\Experimental\RemoteSession\Source\RemoteSession\Private\Channels\RemoteSessionARCameraChannel.cpp
    //                 const int32 NumTriangles = FCircleRasterizeVertexBuffer::NumVertices - 2;
    // 
    // 
    // 
    //                 From VolumetricFog.cpp :D
    // 
    //                 TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> Indices;
    //                 Indices.Empty(NumTriangles * 3);
    // 
    //                 for (int32 TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
    //                 {
    //                     int32 LeadingVertexIndex = TriangleIndex + 2;
    //                     Indices.Add(0);
    //                     Indices.Add(LeadingVertexIndex - 1);
    //                     Indices.Add(LeadingVertexIndex);
    //                 }
    // 
    //                 const uint32 Size = Indices.GetResourceDataSize();
    //                 const uint32 Stride = sizeof(uint16);
    // 
    //                 // Create index buffer. Fill buffer with initial data upon creation
    //                 FRHIResourceCreateInfo CreateInfo(&Indices);
    //                 IndexBufferRHI = RHICreateIndexBuffer(Stride, Size, BUF_Static, CreateInfo);
    //             */
    // 
    // 
    //             FVertexDeclarationRHIRef VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(UFluidSimulationRender::VertexSimulationDataDeclaration);
    //             GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDeclarationRHI;
    //             // Set Pipeline State Object
    //             SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
    // 
    //             // Set Shader Parameters
    //             FFluidSimulationVS::FParameters VSParams;
    //             FFluidSimulationPS::FParameters PSParams;
    // 
    //             SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), VSParams);
    //             SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PSParams);
    // 
    // 
    //             RHICmdList.SetStreamSource(0, InVertexBuffer, 0);
    //             RHICmdList.DrawPrimitive(0, 256 * 256, 1);
    //             // RHICmdList.DrawIndexedPrimitive( ... )
    //         }
    // 
    //         RHICmdList.EndRenderPass();
    //     }
    // }
}