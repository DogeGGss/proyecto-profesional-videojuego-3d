#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pendulo.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class MYPROJECT_API APendulo : public AActor
{
	GENERATED_BODY()

public:
	APendulo();

	virtual void Tick(float DeltaTime) override;

	/** Recalcula el largo del brazo y la posición de la cabeza al editar en el editor. */
	virtual void OnConstruction(const FTransform& Transform) override;

	// --- Componentes ---

	/** Punto fijo del que cuelga todo. Va a la altura del techo. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pendulo")
	USceneComponent* Pivote;

	/** La soga o vara. Sin colisión. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pendulo")
	UStaticMeshComponent* Brazo;

	/** La bola o martillo: es lo que golpea al jugador. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pendulo")
	UStaticMeshComponent* Cabeza;

	// --- Movimiento ---

	/** Qué tan rápido oscila. Más alto = más rápido. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Movimiento")
	float Velocidad = 2.0f;

	/** Amplitud del arco, en grados hacia cada lado. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Movimiento")
	float AnguloMaximo = 60.0f;

	/** Corrimiento del arranque. Poné valores distintos si ponés varios en fila. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Movimiento")
	float Desfase = 0.0f;

	/** Distancia del pivote a la cabeza. El brazo se estira solo para cubrirla. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Movimiento")
	float Largo = 400.0f;

	/** Grosor del brazo. 0.08 queda como una soga fina. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Movimiento")
	float GrosorBrazo = 0.08f;

	/** Si está tildado oscila de costado (Roll); si no, hacia adelante y atrás (Pitch). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Movimiento")
	bool bOscilarDeCostado = false;

	// --- Golpe ---

	/** Empuje horizontal al golpear al jugador. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Golpe")
	float Fuerza = 1200.0f;

	/** Cuánto lo levanta además de empujarlo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Golpe")
	float FuerzaVertical = 400.0f;

	/** Segundos mínimos entre dos golpes, para que no lo empuje en loop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pendulo|Golpe")
	float TiempoEntreGolpes = 1.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void AlSolaparse(UPrimitiveComponent* ComponentePropio, AActor* OtroActor,
		UPrimitiveComponent* OtroComponente, int32 OtroIndice,
		bool bDeBarrido, const FHitResult& Golpe);

private:
	float TiempoUltimoGolpe = -1000.0f;
};