// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "GlobalShader.h"
#include "ShaderCompilerCore.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterStruct.h"

/** Particle simulation draw vertex shader implementation */
class FFluidSimulationPS : public FGlobalShader
{
public:

    DECLARE_GLOBAL_SHADER(FFluidSimulationPS);
    SHADER_USE_PARAMETER_STRUCT(FFluidSimulationPS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
    END_SHADER_PARAMETER_STRUCT()

public:

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
    }

};