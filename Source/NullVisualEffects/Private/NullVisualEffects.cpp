// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "NullVisualEffects.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FNullVisualEffectsModule"

void FNullVisualEffectsModule::StartupModule()
{
    const FString ShaderDirectory = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("NullVisualEffects/Shaders/Private"));
    AddShaderSourceDirectoryMapping(TEXT("/NullVisualEffects"), ShaderDirectory);
}

void FNullVisualEffectsModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNullVisualEffectsModule, NullVisualEffects)