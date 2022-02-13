// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NullVisualEffectsFunctionLibrary.generated.h"

UCLASS()
class NULLVISUALEFFECTS_API UNullVisualEffectsFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /** Copies a vertex buffer */
    static void CopyVertexBuffer(const FVertexBufferRHIRef InSourceBuffer, const FVertexBufferRHIRef InDestinationBuffer);

private:

    /** Copies a vertex buffer render thread implementation */
    static void CopyVertexBuffer_RenderThread(const FVertexBufferRHIRef InSourceBuffer, const FVertexBufferRHIRef InDestinationBuffer, FRHICommandListImmediate& RHICmdList);

};