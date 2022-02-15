// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "FluidSimulation/FluidSimulationActor.h"
#include "FluidSimulation/Render/FluidSimulationRender.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AFluidSimulationActor::AFluidSimulationActor()
    : SimulationGridSize(256)
    , RenderTargetSize(2048)
    , MaterialSlotName(FName(TEXT("M_BaseMaterial")))
    , RenderTargetMaterialParameterName(FName(TEXT("SimulationRT")))
    , FluidRenderTarget(nullptr)
    , MaterialInstanceDynamic(nullptr)
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultStaticMeshRef(TEXT("StaticMesh'/NullVisualEffects/FluidSimulation/SM_FluidSimulation_Plane.SM_FluidSimulation_Plane'"));
    if (DefaultStaticMeshRef.Succeeded())
    {
        DefaultStaticMesh = DefaultStaticMeshRef.Object;
    }

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("FluidSimulationRootComponent"));
    RootComponent->SetMobility(EComponentMobility::Static);

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    StaticMeshComponent->SetupAttachment(RootComponent);
    StaticMeshComponent->SetStaticMesh(DefaultStaticMesh);
    StaticMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    StaticMeshComponent->Mobility = EComponentMobility::Static;
    StaticMeshComponent->SetGenerateOverlapEvents(false);
    StaticMeshComponent->bUseDefaultCollision = true;
}

AFluidSimulationActor::~AFluidSimulationActor()
{
}

void AFluidSimulationActor::BeginPlay()
{
    Super::BeginPlay();
}

void AFluidSimulationActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AFluidSimulationActor::InitResources()
{
    // Temporary
    // FluidRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize, RenderTargetSize, ETextureRenderTargetFormat::RTF_RGBA8, FLinearColor::White);

    FluidSimulationRender = NewObject<UFluidSimulationRender>(this, FName(TEXT("FluidSimulationRender")), RF_Transient);
    FluidSimulationRender->Init(SimulationGridSize);
    FluidRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, SimulationGridSize, SimulationGridSize, ETextureRenderTargetFormat::RTF_RGBA8, FLinearColor::Black);

    if (StaticMeshComponent != nullptr)
    {
        const int32 MaterialIndex = StaticMeshComponent->GetMaterialIndex(MaterialSlotName);

        if (MaterialIndex != INDEX_NONE)
        {
            if (UMaterialInterface* const Material = StaticMeshComponent->GetMaterial(MaterialIndex))
            {
                UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material);

                if (DynamicMaterial == nullptr)
                {
                    DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
                }

                if (DynamicMaterial != nullptr)
                {
                    DynamicMaterial->SetTextureParameterValue(RenderTargetMaterialParameterName, FluidRenderTarget);
                }

                StaticMeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }
}

void AFluidSimulationActor::RegisterBody(const FVector& InCurrentLocation, const FVector& InPreviousLocation, const FVector& InVelocity, const float InStrength)
{
    const FVector& CurrentLocationDelta = GetActorLocation() - InCurrentLocation;
    const FVector& CurrentLocationNormalizedDelta = static_cast<FVector>(CurrentLocationDelta / SimulationGridSize); // #TODO: Implement size squared.

    const FVector& PreviousLocationDelta = GetActorLocation() - InPreviousLocation;
    const FVector& PreviousLocationNormalizedDelta = static_cast<FVector>(PreviousLocationDelta / SimulationGridSize); // #TODO: Implement size squared.

    if (CurrentLocationNormalizedDelta.X <= 1.0f && CurrentLocationNormalizedDelta.X >= -1.0f && CurrentLocationNormalizedDelta.Y <= 1.0f && CurrentLocationNormalizedDelta.Y >= -1.0f)
    {

    }
}

void AFluidSimulationActor::Draw()
{
    FluidSimulationRender->DrawToRenderTarget(FluidRenderTarget);
}
