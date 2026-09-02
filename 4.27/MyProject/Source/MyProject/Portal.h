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

	// ------------------------------------------------------------------
	// Configuración
	// ------------------------------------------------------------------

	/** El portal al que lleva este. Se asigna por instancia, en el nivel. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Portal")
	APortal* PortalDestino;

	/** Material del portal. Debe tener un parámetro de textura llamado "TexturaPortal". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	UMaterialInterface* MaterialPortal;

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

	// --- Rendimiento ---

	/** Resolución de la vista. Más alto es más nítido y más caro. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Rendimiento")
	int32 ResolucionCaptura = 1024;

	/** Más lejos que esto, el portal deja de renderizar la vista. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Rendimiento")
	float DistanciaMaximaRender = 3000.0f;

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