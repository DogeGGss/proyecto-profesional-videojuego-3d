#include "Portal.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Scene.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

	Raiz = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
	RootComponent = Raiz;

	// Mallas del motor, asignadas acá para no tener que hacerlo a mano
	// en el Blueprint cinco veces.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlanoAsset(
		TEXT("/Engine/BasicShapes/Plane.Plane"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CuboAsset(
		TEXT("/Engine/BasicShapes/Cube.Cube"));

	// El eje +X del actor (la flecha) es la cara por la que se entra.
	// La malla Plane mira hacia +Z, así que la giramos para pararla.
	Superficie = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Superficie"));
	Superficie->SetupAttachment(Raiz);
	Superficie->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	Superficie->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (PlanoAsset.Succeeded())
	{
		Superficie->SetStaticMesh(PlanoAsset.Object);
	}

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

	MarcoArriba = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarcoArriba"));
	MarcoAbajo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarcoAbajo"));
	MarcoIzquierda = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarcoIzquierda"));
	MarcoDerecha = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarcoDerecha"));

	UStaticMeshComponent* Barras[] = { MarcoArriba, MarcoAbajo, MarcoIzquierda, MarcoDerecha };
	for (UStaticMeshComponent* Barra : Barras)
	{
		Barra->SetupAttachment(Raiz);
		if (CuboAsset.Succeeded())
		{
			Barra->SetStaticMesh(CuboAsset.Object);
		}
	}

	Captura = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Captura"));
	Captura->SetupAttachment(Raiz);
	// La movemos nosotros cada frame, no queremos que capture sola.
	Captura->bCaptureEveryFrame = false;
	Captura->bCaptureOnMovement = false;

	// HDR y no LDR: FinalColorLDR ya viene tonemapeado y con gamma, y al meterlo
	// en el Emissive del material vuelve a pasar por el tonemapper del pase
	// principal. Ese doble paso lava los colores y levanta los negros.
	// FinalColorHDR aplica todo el post-procesado pero deja el color lineal,
	// así el tonemapper corre una sola vez, igual que para la cámara real.
	// IMPORTANTE: el nodo TextureSampleParameter2D del material tiene que tener
	// Sampler Type = Linear Color, o el shader vuelve a convertir de sRGB algo
	// que ya está en lineal y el color queda mal igual.
	Captura->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;

	DestinoDeRender = nullptr;
	MaterialDinamico = nullptr;
	PersonajeEnZona = nullptr;
	LadoAnterior = 0.f;
	bIgnorarSiguienteEntrada = false;
}

void APortal::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// El Plane del motor mide 100x100. Como lo giramos -90 en Pitch, su eje X
	// local pasa a ser el vertical y el Y sigue siendo el horizontal.
	Superficie->SetRelativeScale3D(FVector(Alto / 100.f, Ancho / 100.f, 1.f));

	// La zona de detección acompaña el tamaño del portal, con un margen para
	// que el jugador entre en ella bastante antes de llegar al plano.
	Deteccion->SetBoxExtent(FVector(
		ProfundidadZona,
		Ancho * 0.5f + 40.f,
		Alto * 0.5f + 40.f));

	// Cubo del motor: 100x100x100 centrado en su origen, así que escala = tamaño / 100.
	// En el espacio local del actor, X es profundidad, Y es ancho y Z es alto.
	const float MitadAncho = Ancho * 0.5f;
	const float MitadAlto = Alto * 0.5f;
	const float Prof = ProfundidadMarco / 100.f;
	const float G = GrosorMarco / 100.f;

	// Las barras horizontales se extienden un grosor a cada lado para tapar
	// las esquinas. Sin eso quedan cuatro huecos en las puntas.
	const float LargoHorizontal = (Ancho + GrosorMarco * 2.f) / 100.f;

	MarcoArriba->SetRelativeScale3D(FVector(Prof, LargoHorizontal, G));
	MarcoArriba->SetRelativeLocation(FVector(0.f, 0.f, MitadAlto + GrosorMarco * 0.5f));

	MarcoAbajo->SetRelativeScale3D(FVector(Prof, LargoHorizontal, G));
	MarcoAbajo->SetRelativeLocation(FVector(0.f, 0.f, -MitadAlto - GrosorMarco * 0.5f));

	MarcoIzquierda->SetRelativeScale3D(FVector(Prof, G, Alto / 100.f));
	MarcoIzquierda->SetRelativeLocation(FVector(0.f, -MitadAncho - GrosorMarco * 0.5f, 0.f));

	MarcoDerecha->SetRelativeScale3D(FVector(Prof, G, Alto / 100.f));
	MarcoDerecha->SetRelativeLocation(FVector(0.f, MitadAncho + GrosorMarco * 0.5f, 0.f));

	const bool bHayMarco = (GrosorMarco > 0.f);
	UStaticMeshComponent* Barras[] = { MarcoArriba, MarcoAbajo, MarcoIzquierda, MarcoDerecha };
	for (UStaticMeshComponent* Barra : Barras)
	{
		Barra->SetVisibility(bHayMarco);
		Barra->SetCollisionEnabled(bHayMarco
			? ECollisionEnabled::QueryAndPhysics
			: ECollisionEnabled::NoCollision);
		if (MaterialMarco != nullptr)
		{
			Barra->SetMaterial(0, MaterialMarco);
		}
	}
}

void APortal::CrearRenderTarget()
{
	int32 AnchoRT = FMath::Max(1, ResolucionCaptura);
	int32 AltoRT = AnchoRT;

	if (GEngine != nullptr && GEngine->GameViewport != nullptr)
	{
		FVector2D TamanoViewport;
		GEngine->GameViewport->GetViewportSize(TamanoViewport);
		if (TamanoViewport.X > 0.f && TamanoViewport.Y > 0.f)
		{
			if (bResolucionDePantalla)
			{
				// Un píxel del render target por cada píxel de pantalla.
				// Es la única forma de que la nitidez sea idéntica a la real.
				AnchoRT = FMath::Max(1, FMath::RoundToInt(TamanoViewport.X));
				AltoRT = FMath::Max(1, FMath::RoundToInt(TamanoViewport.Y));
			}
			else
			{
				// Aunque no sea a resolución completa, el render target tiene que
				// respetar la relación de aspecto de la pantalla: el material lo
				// mapea con ScreenPosition, así que uno cuadrado se estira sobre
				// un viewport panorámico y todo se ve achatado.
				const float Aspecto = TamanoViewport.X / TamanoViewport.Y;
				AltoRT = FMath::Max(1, FMath::RoundToInt(AnchoRT / Aspecto));
			}
		}
	}

	// El formato por defecto es RTF_RGBA16f: float lineal, sin sRGB. Eso es lo
	// que hace falta para que el color HDR de la captura llegue sin alterarse.
	DestinoDeRender = UKismetRenderingLibrary::CreateRenderTarget2D(this, AnchoRT, AltoRT);
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	Deteccion->OnComponentBeginOverlap.AddDynamic(this, &APortal::AlEntrar);
	Deteccion->OnComponentEndOverlap.AddDynamic(this, &APortal::AlSalir);

	if (bFijarExposicion)
	{
		// La captura tiene su propia adaptación de brillo, independiente de la
		// del jugador, y las dos convergen a valores distintos. Sin fijarla, el
		// portal queda más claro u oscuro según hacia dónde estés mirando.
		Captura->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
		Captura->PostProcessSettings.AutoExposureMinBrightness = 1.0f;
		Captura->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
		Captura->PostProcessSettings.AutoExposureMaxBrightness = 1.0f;
	}

	if (MaterialPortal != nullptr)
	{
		CrearRenderTarget();

		MaterialDinamico = UMaterialInstanceDynamic::Create(MaterialPortal, this);
		MaterialDinamico->SetTextureParameterValue(TEXT("TexturaPortal"), DestinoDeRender);
		Superficie->SetMaterial(0, MaterialDinamico);

		Captura->TextureTarget = DestinoDeRender;
	}
}

void APortal::CambiarDestino(APortal* NuevoDestino)
{
	PortalDestino = NuevoDestino;
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
	// Funciona con cualquier rotación de los dos: alcanza con que las dos
	// flechas apunten hacia afuera, hacia donde llega el jugador.
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

	Captura->FOVAngle = Camara->GetFOVAngle();

	// Sin este plano de corte se dibujaría todo lo que está DETRÁS del portal de
	// salida: paredes, objetos intermedios. Es el "oblique near plane clipping"
	// de los tutoriales, que en 4.27 ya viene resuelto.
	Captura->bEnableClipPlane = true;
	Captura->ClipPlaneBase = PortalDestino->GetActorLocation()
		- PortalDestino->GetActorForwardVector() * 2.f;
	Captura->ClipPlaneNormal = PortalDestino->GetActorForwardVector();

	// El portal de salida no debe aparecer dentro de su propia vista: estamos
	// mirando DESDE él, no hacia él. Ocultarlo como actor se lleva también su
	// marco, que es lo que se colaba en la imagen.
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

		// Registramos de qué lado entró, con la misma referencia que usa
		// ComprobarCruce. Si no se restara AjusteCruce acá también, un ajuste
		// positivo podría disparar un cruce falso en el primer frame.
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