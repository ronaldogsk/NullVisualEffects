// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "Library/NullVisualEffectsFunctionLibrary.h"

void UNullVisualEffectsFunctionLibrary::CopyVertexBuffer(const FVertexBufferRHIRef InSourceBuffer, const FVertexBufferRHIRef InDestinationBuffer)
{
    ENQUEUE_RENDER_COMMAND(NullVisualEffectsFunctionLibrary_CopyVertexBuffer)
    (
        [
            InSourceBuffer,
            InDestinationBuffer
        ]
        (FRHICommandListImmediate& RHICmdList)
        {
            CopyVertexBuffer_RenderThread(InSourceBuffer, InDestinationBuffer, RHICmdList);
        }
    );
}

void UNullVisualEffectsFunctionLibrary::CopyVertexBuffer_RenderThread(const FVertexBufferRHIRef InSourceBuffer, const FVertexBufferRHIRef InDestinationBuffer, FRHICommandListImmediate& RHICmdList)
{
    check(IsInRenderingThread());
    QUICK_SCOPE_CYCLE_COUNTER(STAT_NullVisualEffectsFunctionLibrary_CopyVertexBuffer);

    if (InSourceBuffer.IsValid() && InDestinationBuffer.IsValid())
    {
        RHICmdList.CopyVertexBuffer(InSourceBuffer.GetReference(), InDestinationBuffer.GetReference());
    }
}
