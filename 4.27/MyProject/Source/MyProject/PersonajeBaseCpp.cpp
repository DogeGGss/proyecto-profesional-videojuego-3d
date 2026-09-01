// Fill out your copyright notice in the Description page of Project Settings.
#include "PersonajeBaseCpp.h"
#include "Components/SphereComponent.h"
#include "MultiplayerGameMode.h"
#include "TagPlayerState.h"

APersonajeBaseCpp::APersonajeBaseCpp()
{
    PrimaryActorTick.bCanEverTick = true;

    AreaMancha = CreateDefaultSubobject<USphereComponent>(TEXT("AreaMancha"));
    AreaMancha->SetupAttachment(RootComponent);
    AreaMancha->SetSphereRadius(150.f);
    AreaMancha->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    AreaMancha->SetGenerateOverlapEvents(true);
}

void APersonajeBaseCpp::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // HasAuthority() asegura que esta decisión crítica solo la tome el servidor
    if (!HasAuthority()) return;

    APersonajeBaseCpp* OtroPersonaje = Cast<APersonajeBaseCpp>(OtherActor);
    if (OtroPersonaje && OtroPersonaje != this)
    {
        ATagPlayerState* MiPS = GetPlayerState<ATagPlayerState>();
        ATagPlayerState* OtroPS = OtroPersonaje->GetPlayerState<ATagPlayerState>();

        if (MiPS && OtroPS)
        {
            if (MiPS->bEsMancha && !OtroPS->bEsMancha)
            {
                AMultiplayerGameMode* GM = Cast<AMultiplayerGameMode>(GetWorld()->GetAuthGameMode());
                if (GM)
                {
                    GM->OtorgarPunto(MiPS);
                }
            }
        }
    }
}

void APersonajeBaseCpp::BeginPlay()
{
    Super::BeginPlay();

    // Al vincular el evento aquí, obligamos al motor a registrar el choque en tiempo real
    if (HasAuthority())
    {
        AreaMancha->OnComponentBeginOverlap.AddDynamic(this, &APersonajeBaseCpp::OnOverlapBegin);
    }
}

void APersonajeBaseCpp::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APersonajeBaseCpp::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}