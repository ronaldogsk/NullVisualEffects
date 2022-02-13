// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "GlobalShader.h"
#include "ShaderCompilerCore.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterStruct.h"

class FFluidSimulationCS : public FGlobalShader
{
public:

    DECLARE_GLOBAL_SHADER(FFluidSimulationCS);
    SHADER_USE_PARAMETER_STRUCT(FFluidSimulationCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_UAV(RWBuffer<float>, FluidData)
        SHADER_PARAMETER(float, DeltaTime)
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
        OutEnvironment.SetDefine(TEXT("CG_SIZE_X"), 32);
    }
};
