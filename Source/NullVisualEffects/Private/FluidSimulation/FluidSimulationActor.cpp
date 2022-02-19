// Copyright (C) Ronaldo Veloso. All Rights Reserved.

#include "FluidSimulation/FluidSimulationActor.h"
#include "FluidSimulation/Render/FluidSimulationRender.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

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

    InitResources();
}

void AFluidSimulationActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AFluidSimulationActor::InitResources()
{
    // Temporary
    // FluidRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize, RenderTargetSize, ETextureRenderTargetFormat::RTF_RGBA8, FLinearColor::White);

    FluidRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, SimulationGridSize, SimulationGridSize, ETextureRenderTargetFormat::RTF_RGBA8, FLinearColor::Black);
    FluidSimulationRender = NewObject<UFluidSimulationRender>(this, FName(TEXT("FluidSimulationRender")), RF_Transient);
    FluidSimulationRender->SetRenderTarget(FluidRenderTarget);
    FluidSimulationRender->Init(SimulationGridSize);

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

void AFluidSimulationActor::RegisterBody(const FVector& InCurrentLocation, const FVector& InPreviousLocation, const FVector& InVelocity, const float InRadius, const float InStrength)
{
    FVector BoundsOrigin = FVector::ZeroVector;
    FVector BoundsBoxExtent = FVector::ZeroVector;
    GetActorBounds(false, BoundsOrigin, BoundsBoxExtent, false);

    const FVector& CurrentLocationDelta = (BoundsOrigin - InCurrentLocation) / BoundsBoxExtent;
    const FVector& CurrentLocationNormalizedDelta = static_cast<FVector>(CurrentLocationDelta / SimulationGridSize); // #TODO: Implement size squared.

    const FVector& PreviousLocationDelta = BoundsOrigin - InPreviousLocation;
    const FVector& PreviousLocationNormalizedDelta = static_cast<FVector>(PreviousLocationDelta / SimulationGridSize); // #TODO: Implement size squared.

    const FVector& Radius = FVector(InRadius, InRadius, InRadius) / BoundsBoxExtent;

    if (CurrentLocationNormalizedDelta.X <= 1.0f && CurrentLocationNormalizedDelta.X >= -1.0f && CurrentLocationNormalizedDelta.Y <= 1.0f && CurrentLocationNormalizedDelta.Y >= -1.0f)
    {
        FluidSimulationRender->AddVelocityDensity(CurrentLocationDelta,/* InVelocity * InStrength*/ FVector::OneVector, Radius.X, 1.0f);
    }

    UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%f %f %f"), CurrentLocationDelta.X, CurrentLocationDelta.Y, CurrentLocationDelta.Z));

#if !UE_BUILD_SHIPPING

    DrawDebugDirectionalArrow(GetWorld(), InPreviousLocation, InCurrentLocation, 0.0f, FColor::Red, false, 1.0f, 0, 10.0f);
#endif
}

void AFluidSimulationActor::Draw()
{
    FluidSimulationRender->DrawToRenderTarget(FluidRenderTarget);
}
