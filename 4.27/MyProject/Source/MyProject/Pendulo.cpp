#include "Pendulo.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"

APendulo::APendulo()
{
	PrimaryActorTick.bCanEverTick = true;

	// El pivote es el punto fijo: todo lo demás cuelga de acá y rota alrededor suyo.
	Pivote = CreateDefaultSubobject<USceneComponent>(TEXT("Pivote"));
	RootComponent = Pivote;

	// El brazo/soga. Sin colisión, para que el jugador no choque contra el cable.
	Brazo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Brazo"));
	Brazo->SetupAttachment(Pivote);
	Brazo->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// La cabeza: lo que efectivamente golpea al jugador.
	Cabeza = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cabeza"));
	Cabeza->SetupAttachment(Pivote);
	Cabeza->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Cabeza->SetCollisionResponseToAllChannels(ECR_Overlap);
	Cabeza->SetGenerateOverlapEvents(true);
}

void APendulo::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// La cabeza cuelga a "Largo" unidades por debajo del pivote.
	Cabeza->SetRelativeLocation(FVector(0.f, 0.f, -Largo));

	// Medimos la malla del brazo: altura y dónde tiene su origen.
	float AlturaMalla = 100.f;
	float CentroMalla = 0.f;
	if (Brazo->GetStaticMesh() != nullptr)
	{
		const FBoxSphereBounds Limites = Brazo->GetStaticMesh()->GetBounds();
		AlturaMalla = Limites.BoxExtent.Z * 2.f;
		CentroMalla = Limites.Origin.Z;

		if (AlturaMalla <= KINDA_SMALL_NUMBER)
		{
			AlturaMalla = 100.f;
		}
	}

	const float Escala = Largo / AlturaMalla;
	Brazo->SetRelativeScale3D(FVector(GrosorBrazo, GrosorBrazo, Escala));

	// Queremos que el CENTRO del cilindro quede a mitad de camino (-Largo/2).
	// Si la malla tiene el origen en la base en vez de en el centro, hay que
	// compensar ese corrimiento, ya escalado.
	Brazo->SetRelativeLocation(FVector(0.f, 0.f, -Largo * 0.5f - CentroMalla * Escala));
}

void APendulo::BeginPlay()
{
	Super::BeginPlay();

	Cabeza->OnComponentBeginOverlap.AddDynamic(this, &APendulo::AlSolaparse);
}

void APendulo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Sin() devuelve entre -1 y 1, así que el ángulo va de -AnguloMaximo a +AnguloMaximo.
	// Al depender del tiempo de juego y no de la física, el recorrido es siempre
	// idéntico: dos corridas del circuito dan el mismo resultado y el récord es justo.
	const float Tiempo = GetWorld()->GetTimeSeconds();
	const float Angulo = FMath::Sin(Tiempo * Velocidad + Desfase) * AnguloMaximo;

	const FRotator NuevaRotacion = bOscilarDeCostado
		? FRotator(0.f, 0.f, Angulo)   // Roll: oscila de costado
		: FRotator(Angulo, 0.f, 0.f);  // Pitch: oscila hacia adelante y atrás

	Pivote->SetRelativeRotation(NuevaRotacion);
}

void APendulo::AlSolaparse(UPrimitiveComponent* ComponentePropio, AActor* OtroActor,
	UPrimitiveComponent* OtroComponente, int32 OtroIndice,
	bool bDeBarrido, const FHitResult& Golpe)
{
	ACharacter* Personaje = Cast<ACharacter>(OtroActor);
	if (Personaje == nullptr)
	{
		return;
	}

	// Cooldown: sin esto, mientras el jugador esté dentro de la bola lo relanza sin parar.
	const float Ahora = GetWorld()->GetTimeSeconds();
	if (Ahora - TiempoUltimoGolpe < TiempoEntreGolpes)
	{
		return;
	}
	TiempoUltimoGolpe = Ahora;

	// Dirección: desde la cabeza hacia el jugador, o sea que lo aleja de la bola.
	// Si lo toca desde la izquierda sale despedido a la derecha, y viceversa,
	// sin necesidad de saber hacia dónde iba el péndulo.
	FVector Direccion = Personaje->GetActorLocation() - Cabeza->GetComponentLocation();
	Direccion.Z = 0.f;
	Direccion.Normalize();

	const FVector Impulso = Direccion * Fuerza + FVector(0.f, 0.f, FuerzaVertical);

	// LaunchCharacter es la forma correcta de empujar un Character: el Character
	// Movement Component ignora los impulsos de física normales.
	Personaje->LaunchCharacter(Impulso, true, true);
}