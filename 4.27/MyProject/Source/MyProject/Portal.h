#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UArrowComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class ACharacter;

UCLASS()
class MYPROJECT_API APortal : public AActor
{
	GENERATED_BODY()

public:
	APortal();

	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// ------------------------------------------------------------------
	// Componentes
	// ------------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	USceneComponent* Raiz;

	/** La superficie visible del portal. Acá se dibuja la vista del otro lado. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UStaticMeshComponent* Superficie;

	/** Volumen que detecta al jugador acercándose. No define el punto de cruce. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UBoxComponent* Deteccion;

	/** Solo para el editor: indica la cara de entrada del portal. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UArrowComponent* Flecha;

	/** Cámara que renderiza lo que se ve desde el portal de salida. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	USceneCaptureComponent2D* Captura;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Marco")
	UStaticMeshComponent* MarcoArriba;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Marco")
	UStaticMeshComponent* MarcoAbajo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Marco")
	UStaticMeshComponent* MarcoIzquierda;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Marco")
	UStaticMeshComponent* MarcoDerecha;

	// ------------------------------------------------------------------
	// Configuración
	// ------------------------------------------------------------------

	/** El portal al que lleva este. Se asigna por instancia, en el nivel. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Portal")
	APortal* PortalDestino;

	/** Material del portal. Debe tener un parámetro de textura llamado "TexturaPortal",
	 *  con Sampler Type en Linear Color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UMaterialInterface* MaterialPortal;

	/** Reapunta este portal a otro destino en tiempo de ejecución.
	 *  Llamalo cuando el jugador NO esté mirando el portal, o va a ver el corte. */
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void CambiarDestino(APortal* NuevoDestino);

	// --- Forma ---

	/** Ancho del portal, en unidades de Unreal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Forma")
	float Ancho = 200.0f;

	/** Alto del portal, en unidades de Unreal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Forma")
	float Alto = 300.0f;

	/** Desde qué distancia el portal empieza a vigilar al jugador. Si es muy
	 *  chica, corriendo rápido se puede atravesar entre dos frames sin detectarse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Forma")
	float ProfundidadZona = 150.0f;

	/** Corre el punto exacto del teletransporte respecto del plano del portal.
	 *  Negativo lo retrasa (cruzás más adentro), positivo lo adelanta. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Forma")
	float AjusteCruce = 0.0f;

	// --- Marco ---

	/** Grosor de las barras del marco. En 0, el marco desaparece. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Marco")
	float GrosorMarco = 20.0f;

	/** Cuánto sobresale el marco hacia adelante y hacia atrás. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Marco")
	float ProfundidadMarco = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Marco")
	UMaterialInterface* MaterialMarco;

	// --- Rendimiento ---

	/** La vista del portal se renderiza exactamente a la resolución de la
	 *  pantalla. Es lo que la hace verse igual de nítida que la vista real,
	 *  y también es lo más caro. Apagalo para builds de Android. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Rendimiento")
	bool bResolucionDePantalla = true;

	/** Ancho de la vista cuando bResolucionDePantalla está desactivado.
	 *  El alto se calcula solo, respetando la relación de aspecto de la pantalla. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Rendimiento",
		meta = (EditCondition = "!bResolucionDePantalla"))
	int32 ResolucionCaptura = 1024;

	/** Más lejos que esto, el portal deja de renderizar la vista. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Rendimiento")
	float DistanciaMaximaRender = 3000.0f;

	/** Fija la exposición de la captura para que no se aclare u oscurezca
	 *  respecto de la vista real. Requiere que el proyecto tenga la exposición
	 *  automática apagada, o la cámara del jugador va a seguir variando sola. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Rendimiento")
	bool bFijarExposicion = false;

	/** Lleva un transform del espacio de este portal al del portal de salida. */
	FTransform ConvertirAlDestino(const FTransform& EnEsteEspacio) const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void AlEntrar(UPrimitiveComponent* ComponentePropio, AActor* OtroActor,
		UPrimitiveComponent* OtroComponente, int32 OtroIndice,
		bool bDeBarrido, const FHitResult& Golpe);

	UFUNCTION()
	void AlSalir(UPrimitiveComponent* ComponentePropio, AActor* OtroActor,
		UPrimitiveComponent* OtroComponente, int32 OtroIndice);

private:
	void CrearRenderTarget();
	void ActualizarCaptura();
	void ComprobarCruce();
	void Teletransportar(ACharacter* Personaje);

	/** El transform del portal de salida girado 180° sobre su eje vertical. */
	FTransform ObtenerSalidaGirada() const;

	UPROPERTY()
	UTextureRenderTarget2D* DestinoDeRender;

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialDinamico;

	UPROPERTY()
	ACharacter* PersonajeEnZona;

	/** De qué lado del plano estaba el jugador el frame anterior. */
	float LadoAnterior;

	/** Evita que el portal de salida te devuelva apenas llegás. */
	bool bIgnorarSiguienteEntrada;
};