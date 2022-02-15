// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "GlobalShader.h"
#include "ShaderCompilerCore.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterStruct.h"

class FFluidSimulationDrawCS : public FGlobalShader
{
public:

    DECLARE_GLOBAL_SHADER(FFluidSimulationDrawCS);
    SHADER_USE_PARAMETER_STRUCT(FFluidSimulationDrawCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_UAV(RWTexture2D<float>, OutTexture)
        SHADER_PARAMETER_UAV(RWBuffer<float>, FluidData)
        SHADER_PARAMETER(int32, SimulationGridSize)
        SHADER_PARAMETER(float, SimulationGridSizeRecip)
    END_SHADER_PARAMETER_STRUCT()

public:

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& InParameters)
    {
        return IsFeatureLevelSupported(InParameters.Platform, ERHIFeatureLevel::SM5);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
    }
};
