// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#pragma once

#include "FluidSimulation/Render/FluidSimulationRender.h"
#include "GlobalShader.h"
#include "ShaderCompilerCore.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterStruct.h"

class FFluidSimulationAddInputCS : public FGlobalShader
{
public:

    DECLARE_GLOBAL_SHADER(FFluidSimulationAddInputCS);
    SHADER_USE_PARAMETER_STRUCT(FFluidSimulationAddInputCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_UAV(RWBuffer<float>, CurrentFluidData)
        SHADER_PARAMETER_UAV(StructuredBuffer<FFluidCellInputData>, ForcesDencityData)
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
    }
};
