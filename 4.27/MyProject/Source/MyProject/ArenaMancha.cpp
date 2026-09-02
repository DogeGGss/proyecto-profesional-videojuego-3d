#include "ArenaMancha.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/StaticMesh.h"

AArenaMancha::AArenaMancha()
{
	PrimaryActorTick.bCanEverTick = false;

	Bloques = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Bloques"));
	RootComponent = Bloques;
	Bloques->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Bloques->SetCollisionProfileName(TEXT("BlockAll"));
	// Movable evita advertencias al regenerar instancias desde OnConstruction.
	Bloques->SetMobility(EComponentMobility::Movable);

	InicioA = CreateDefaultSubobject<UArrowComponent>(TEXT("InicioA"));
	InicioA->SetupAttachment(Bloques);

	InicioB = CreateDefaultSubobject<UArrowComponent>(TEXT("InicioB"));
	InicioB->SetupAttachment(Bloques);
}

void AArenaMancha::AgregarCaja(const FVector& Centro, const FVector& Tamano, const FRotator& Rotacion)
{
	if (TamanoMallaBase.IsNearlyZero())
	{
		return;
	}

	// Convertimos el tamaño deseado en escala, según cuánto mide la malla base.
	const FVector Escala(
		Tamano.X / TamanoMallaBase.X,
		Tamano.Y / TamanoMallaBase.Y,
		Tamano.Z / TamanoMallaBase.Z);

	Bloques->AddInstance(FTransform(Rotacion, Centro, Escala));
}

void AArenaMancha::AgregarRampa(const FVector& Desde, const FVector& Hasta)
{
	const FVector Delta = Hasta - Desde;
	const float LargoTotal = Delta.Size();
	if (LargoTotal <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Rotation() devuelve el rotador cuyo eje +X apunta en la dirección del
	// vector: nos resuelve inclinación y giro de una, sin trigonometría a mano.
	AgregarCaja((Desde + Hasta) * 0.5f,
		FVector(LargoTotal, AnchoRampa, Espesor),
		Delta.Rotation());
}

void AArenaMancha::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Bloques->GetStaticMesh() == nullptr)
	{
		Bloques->ClearInstances();
		return;
	}

	// Medimos la malla una vez: así funciona con el cubo de BasicShapes,
	// el del StarterContent o cualquier otro.
	const FBoxSphereBounds Limites = Bloques->GetStaticMesh()->GetBounds();
	TamanoMallaBase = Limites.BoxExtent * 2.f;
	if (TamanoMallaBase.IsNearlyZero())
	{
		TamanoMallaBase = FVector(100.f, 100.f, 100.f);
	}

	Bloques->ClearInstances();

	const float Mitad = LadoArena * 0.5f;

	// --- Piso ---
	AgregarCaja(FVector(0.f, 0.f, -Espesor * 0.5f),
		FVector(LadoArena, LadoArena, Espesor));

	// --- Muros perimetrales ---
	// Cierran la arena: sin vacío al que tirarse, el evasor no puede usar la
	// caída como escape gratis cuando lo acorralan.
	const float LargoMuro = LadoArena + Espesor * 2.f;
	const float CentroMuro = Mitad + Espesor * 0.5f;
	const float AltoMuroCentro = AlturaMuro * 0.5f;

	AgregarCaja(FVector(CentroMuro, 0.f, AltoMuroCentro), FVector(Espesor, LargoMuro, AlturaMuro));
	AgregarCaja(FVector(-CentroMuro, 0.f, AltoMuroCentro), FVector(Espesor, LargoMuro, AlturaMuro));
	AgregarCaja(FVector(0.f, CentroMuro, AltoMuroCentro), FVector(LargoMuro, Espesor, AlturaMuro));
	AgregarCaja(FVector(0.f, -CentroMuro, AltoMuroCentro), FVector(LargoMuro, Espesor, AlturaMuro));

	// --- Bloque central, hecho de cuatro cubos con una cruz de pasillos ---
	// Esta es la pieza clave del diseño: el evasor corre en círculo alrededor,
	// y el mancha decide si lo sigue o cruza por el medio para interceptarlo.
	const float LadoSub = FMath::Max((LadoBloqueCentral - AnchoPasillo) * 0.5f, 50.f);
	const float Corrimiento = (AnchoPasillo + LadoSub) * 0.5f;

	for (int32 sx = -1; sx <= 1; sx += 2)
	{
		for (int32 sy = -1; sy <= 1; sy += 2)
		{
			AgregarCaja(
				FVector(sx * Corrimiento, sy * Corrimiento, AlturaBloqueCentral * 0.5f),
				FVector(LadoSub, LadoSub, AlturaBloqueCentral));
		}
	}

	// --- Plataforma superior ---
	// Tapa la cruz central, así los pasillos quedan como túneles y arriba hay
	// un segundo nivel desde el que el mancha puede tirarse a cortar camino.
	const float AlturaPlataforma = AlturaBloqueCentral + Espesor * 0.5f;
	AgregarCaja(FVector(0.f, 0.f, AlturaPlataforma),
		FVector(LadoBloqueCentral, LadoBloqueCentral, Espesor));

	// --- Rampas en dos esquinas opuestas ---
	// Opuestas y no adyacentes: la arena queda simétrica al girarla 180°,
	// que es la simetría que importa para que ningún jugador tenga ventaja.
	const float BordeCentro = LadoBloqueCentral * 0.5f;
	const float Diagonal = FMath::Sqrt(2.f) * 0.5f;
	const float AlcanceRampa = BordeCentro + LargoRampa * Diagonal;

	AgregarRampa(
		FVector(AlcanceRampa, AlcanceRampa, 0.f),
		FVector(BordeCentro, BordeCentro, AlturaBloqueCentral + Espesor));

	AgregarRampa(
		FVector(-AlcanceRampa, -AlcanceRampa, 0.f),
		FVector(-BordeCentro, -BordeCentro, AlturaBloqueCentral + Espesor));

	// --- Marcadores de salida ---
	// En las dos esquinas que NO tienen rampa, mirando al centro.
	const float PosInicio = Mitad - 350.f;

	InicioA->SetRelativeLocation(FVector(PosInicio, -PosInicio, 120.f));
	InicioA->SetRelativeRotation(FVector(-1.f, 1.f, 0.f).Rotation());

	InicioB->SetRelativeLocation(FVector(-PosInicio, PosInicio, 120.f));
	InicioB->SetRelativeRotation(FVector(1.f, -1.f, 0.f).Rotation());
}