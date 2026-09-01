#include "Barredora.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"

ABarredora::ABarredora()
{
	PrimaryActorTick.bCanEverTick = true;

	// El pivote es el eje de giro. Va en el centro del camino, a la altura
	// a la que querés que barra la barra.
	Pivote = CreateDefaultSubobject<USceneComponent>(TEXT("Pivote"));
	RootComponent = Pivote;

	// El brazo sale horizontal desde el eje hacia +X y gira con él.
	Brazo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Brazo"));
	Brazo->SetupAttachment(Pivote);
	Brazo->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Brazo->SetCollisionResponseToAllChannels(ECR_Overlap);
	Brazo->SetGenerateOverlapEvents(true);
}

void ABarredora::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Medimos la malla a lo largo de X: cuánto mide y dónde tiene el origen.
	// Así funciona con un cubo centrado, con uno de origen en la punta, o
	// con cualquier malla propia.
	float LargoMalla = 100.f;
	float CentroMalla = 0.f;
	if (Brazo->GetStaticMesh() != nullptr)
	{
		const FBoxSphereBounds Limites = Brazo->GetStaticMesh()->GetBounds();
		LargoMalla = Limites.BoxExtent.X * 2.f;
		CentroMalla = Limites.Origin.X;

		if (LargoMalla <= KINDA_SMALL_NUMBER)
		{
			LargoMalla = 100.f;
		}
	}

	const float Escala = Largo / LargoMalla;
	Brazo->SetRelativeScale3D(FVector(Escala, GrosorBrazo, GrosorBrazo));

	// Queremos que el brazo vaya del eje (X=0) hasta X=Largo, o sea con su
	// centro en Largo/2, compensando el corrimiento del origen de la malla.
	Brazo->SetRelativeLocation(FVector(Largo * 0.5f - CentroMalla * Escala, 0.f, 0.f));
}

void ABarredora::BeginPlay()
{
	Super::BeginPlay();

	Brazo->OnComponentBeginOverlap.AddDynamic(this, &ABarredora::AlSolaparse);
}

void ABarredora::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Giro continuo en Yaw. Igual que el péndulo, depende del tiempo de juego
	// y no de la física, así que el recorrido es idéntico en cada corrida.
	// El Fmod evita que el ángulo crezca sin límite en partidas largas.
	const float Tiempo = GetWorld()->GetTimeSeconds();
	const float Angulo = FMath::Fmod(AnguloInicial + Tiempo * VelocidadGrados, 360.f);

	Pivote->SetRelativeRotation(FRotator(0.f, Angulo, 0.f));
}

void ABarredora::AlSolaparse(UPrimitiveComponent* ComponentePropio, AActor* OtroActor,
	UPrimitiveComponent* OtroComponente, int32 OtroIndice,
	bool bDeBarrido, const FHitResult& Golpe)
{
	ACharacter* Personaje = Cast<ACharacter>(OtroActor);
	if (Personaje == nullptr)
	{
		return;
	}

	const float Ahora = GetWorld()->GetTimeSeconds();
	if (Ahora - TiempoUltimoGolpe < TiempoEntreGolpes)
	{
		return;
	}
	TiempoUltimoGolpe = Ahora;

	// Radial: del eje hacia el jugador (hacia afuera).
	FVector Radial = Personaje->GetActorLocation() - GetActorLocation();
	Radial.Z = 0.f;
	Radial.Normalize();

	// Tangencial: perpendicular al radio, o sea la dirección en la que se está
	// moviendo la barra en ese punto. Es lo que hace que el golpe se sienta
	// como un barrido y no como un empujón hacia afuera.
	FVector Tangencial = FVector::CrossProduct(FVector::UpVector, Radial);
	if (VelocidadGrados < 0.f)
	{
		Tangencial *= -1.f;
	}

	const FVector Direccion = (Tangencial * (1.f - EmpujeHaciaAfuera)
		+ Radial * EmpujeHaciaAfuera).GetSafeNormal();

	const FVector Impulso = Direccion * Fuerza + FVector(0.f, 0.f, FuerzaVertical);

	Personaje->LaunchCharacter(Impulso, true, true);
}