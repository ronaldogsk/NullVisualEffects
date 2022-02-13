// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "FluidSimulation/Render/FluidSimulationCS.h"

IMPLEMENT_GLOBAL_SHADER(FFluidSimulationCS, "/NullVisualEffects/FluidSimulation/FluidSimulationCS.usf", "MainCS", SF_Compute);