#include "Portal.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

	Raiz = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
	RootComponent = Raiz;

	// El eje +X del actor (la flecha roja) es la cara por la que se entra.
	// La malla Plane de Unreal mira hacia +Z, así que la giramos para pararla.
	Superficie = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Superficie"));
	Superficie->SetupAttachment(Raiz);
	Superficie->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	Superficie->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Deteccion = CreateDefaultSubobject<UBoxComponent>(TEXT("Deteccion"));
	Deteccion->SetupAttachment(Raiz);
	Deteccion->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Deteccion->SetCollisionResponseToAllChannels(ECR_Overlap);
	Deteccion->SetGenerateOverlapEvents(true);

	// Ayuda visual para no equivocarse al orientar los portales en el nivel.
	Flecha = CreateDefaultSubobject<UArrowComponent>(TEXT("Flecha"));
	Flecha->SetupAttachment(Raiz);
	Flecha->ArrowSize = 2.0f;
	Flecha->SetArrowColor(FColor::Cyan);
	Flecha->SetHiddenInGame(true);

	Captura = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Captura"));
	Captura->SetupAttachment(Raiz);
	// La movemos nosotros cada frame, no queremos que capture sola.
	Captura->bCaptureEveryFrame = false;
	Captura->bCaptureOnMovement = false;
	Captura->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	DestinoDeRender = nullptr;
	MaterialDinamico = nullptr;
	PersonajeEnZona = nullptr;
	LadoAnterior = 0.f;
	bIgnorarSiguienteEntrada = false;
}

void APortal::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// El Plane de Unreal mide 100x100. Como lo giramos -90 en Pitch, su eje X
	// local pasa a ser el vertical y el Y sigue siendo el horizontal.
	Superficie->SetRelativeScale3D(FVector(Alto / 100.f, Ancho / 100.f, 1.f));

	// La zona de detección acompaña el tamaño del portal, con un margen para
	// que el jugador entre en ella bastante antes de llegar al plano.
	Deteccion->SetBoxExtent(FVector(
		ProfundidadZona,
		Ancho * 0.5f + 40.f,
		Alto * 0.5f + 40.f));
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	Deteccion->OnComponentBeginOverlap.AddDynamic(this, &APortal::AlEntrar);
	Deteccion->OnComponentEndOverlap.AddDynamic(this, &APortal::AlSalir);

	if (MaterialPortal != nullptr)
	{
		DestinoDeRender = UKismetRenderingLibrary::CreateRenderTarget2D(
			this, ResolucionCaptura, ResolucionCaptura);

		MaterialDinamico = UMaterialInstanceDynamic::Create(MaterialPortal, this);
		MaterialDinamico->SetTextureParameterValue(TEXT("TexturaPortal"), DestinoDeRender);
		Superficie->SetMaterial(0, MaterialDinamico);

		Captura->TextureTarget = DestinoDeRender;
	}
}

FTransform APortal::ObtenerSalidaGirada() const
{
	// El portal de salida girado 180° sobre su eje vertical: uno entra mirando
	// HACIA este portal, y sale mirando HACIA AFUERA del otro.
	return FTransform(FRotator(0.f, 180.f, 0.f)) * PortalDestino->GetActorTransform();
}

FTransform APortal::ConvertirAlDestino(const FTransform& EnEsteEspacio) const
{
	if (PortalDestino == nullptr)
	{
		return EnEsteEspacio;
	}

	// Lo que era relativo a este portal pasa a serlo respecto del de salida.
	// Esto funciona con cualquier rotación de los dos portales: alcanza con que
	// las dos flechas apunten hacia afuera, hacia donde llega el jugador.
	const FTransform Relativo = EnEsteEspacio.GetRelativeTransform(GetActorTransform());
	return Relativo * ObtenerSalidaGirada();
}

void APortal::ActualizarCaptura()
{
	if (PortalDestino == nullptr || DestinoDeRender == nullptr)
	{
		return;
	}

	APlayerCameraManager* Camara = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (Camara == nullptr)
	{
		return;
	}

	// La cámara de captura se ubica donde estaría el ojo del jugador si ya
	// hubiera cruzado: misma posición relativa, pero respecto del otro portal.
	const FTransform TransformCamara(Camara->GetCameraRotation(), Camara->GetCameraLocation());
	const FTransform TransformCaptura = ConvertirAlDestino(TransformCamara);

	Captura->SetWorldLocationAndRotation(
		TransformCaptura.GetLocation(), TransformCaptura.GetRotation());

	// Mismo campo de visión que el jugador, o la vista no calza con la pantalla.
	Captura->FOVAngle = Camara->GetFOVAngle();

	// Sin este plano de corte se dibujaría todo lo que está DETRÁS del portal de
	// salida: paredes, el propio marco, objetos intermedios. Es lo que en los
	// tutoriales aparece como "oblique near plane clipping", y en 4.27 viene resuelto.
	Captura->bEnableClipPlane = true;
	Captura->ClipPlaneBase = PortalDestino->GetActorLocation()
		- PortalDestino->GetActorForwardVector() * 2.f;
	Captura->ClipPlaneNormal = PortalDestino->GetActorForwardVector();
	Captura->HiddenActors.Empty();
	Captura->HiddenActors.Add(PortalDestino);
	Captura->CaptureScene();
}

void APortal::ComprobarCruce()
{
	if (PersonajeEnZona == nullptr || PortalDestino == nullptr)
	{
		return;
	}

	// Distancia con signo al plano del portal: positiva delante, negativa detrás.
	// AjusteCruce corre ese plano hacia adelante o hacia atrás.
	const FVector Hacia = PersonajeEnZona->GetActorLocation() - GetActorLocation();
	const float Lado = FVector::DotProduct(Hacia, GetActorForwardVector()) - AjusteCruce;

	// Si el signo cambió, cruzó el plano justo en este frame.
	if (LadoAnterior > 0.f && Lado <= 0.f)
	{
		Teletransportar(PersonajeEnZona);
		return;
	}

	LadoAnterior = Lado;
}

void APortal::Teletransportar(ACharacter* Personaje)
{
	if (Personaje == nullptr || PortalDestino == nullptr)
	{
		return;
	}

	// El portal de salida no debe reaccionar a esta llegada y devolvernos.
	PortalDestino->bIgnorarSiguienteEntrada = true;

	const FTransform Destino = ConvertirAlDestino(Personaje->GetActorTransform());
	Personaje->SetActorLocationAndRotation(Destino.GetLocation(), Destino.GetRotation());

	// La cámara: misma conversión aplicada a la rotación del control.
	if (AController* Control = Personaje->GetController())
	{
		const FTransform RotacionControl(Control->GetControlRotation());
		Control->SetControlRotation(ConvertirAlDestino(RotacionControl).Rotator());
	}

	// La velocidad se conserva rotada: si entrás corriendo, salís corriendo en
	// la dirección equivalente del otro lado. Sin esto el portal frena al jugador.
	if (UCharacterMovementComponent* Movimiento = Personaje->GetCharacterMovement())
	{
		const FVector VelocidadLocal =
			GetActorTransform().InverseTransformVector(Movimiento->Velocity);
		Movimiento->Velocity = ObtenerSalidaGirada().TransformVector(VelocidadLocal);
	}

	PersonajeEnZona = nullptr;
	LadoAnterior = 0.f;
}

void APortal::AlEntrar(UPrimitiveComponent* ComponentePropio, AActor* OtroActor,
	UPrimitiveComponent* OtroComponente, int32 OtroIndice,
	bool bDeBarrido, const FHitResult& Golpe)
{
	if (bIgnorarSiguienteEntrada)
	{
		bIgnorarSiguienteEntrada = false;
		return;
	}

	if (ACharacter* Personaje = Cast<ACharacter>(OtroActor))
	{
		PersonajeEnZona = Personaje;

		// Registramos de qué lado entró, para detectar el cruce después.
		const FVector Hacia = Personaje->GetActorLocation() - GetActorLocation();
		LadoAnterior = FVector::DotProduct(Hacia, GetActorForwardVector()) - AjusteCruce;
	}
}

void APortal::AlSalir(UPrimitiveComponent* ComponentePropio, AActor* OtroActor,
	UPrimitiveComponent* OtroComponente, int32 OtroIndice)
{
	if (Cast<ACharacter>(OtroActor) == PersonajeEnZona)
	{
		PersonajeEnZona = nullptr;
		bIgnorarSiguienteEntrada = false;
	}
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Cada portal visible cuesta un render completo de la escena por frame,
	// así que solo capturamos si el jugador está cerca.
	if (APlayerCameraManager* Camara = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		if (FVector::Dist(Camara->GetCameraLocation(), GetActorLocation()) <= DistanciaMaximaRender)
		{
			ActualizarCaptura();
		}
	}

	ComprobarCruce();
}