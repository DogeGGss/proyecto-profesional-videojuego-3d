#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Barredora.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class MYPROJECT_API ABarredora : public AActor
{
	GENERATED_BODY()

public:
	ABarredora();

	virtual void Tick(float DeltaTime) override;

	/** Recalcula el largo del brazo al editar en el editor. */
	virtual void OnConstruction(const FTransform& Transform) override;

	// --- Componentes ---

	/** Eje vertical alrededor del cual gira el brazo. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barredora")
	USceneComponent* Pivote;

	/** La barra horizontal que barre. Es lo que golpea al jugador. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barredora")
	UStaticMeshComponent* Brazo;

	// --- Movimiento ---

	/** Grados por segundo. Negativo invierte el sentido de giro. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barredora|Movimiento")
	float VelocidadGrados = 90.0f;

	/** Ángulo de arranque. Poné valores distintos si ponés varias en fila. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barredora|Movimiento")
	float AnguloInicial = 0.0f;

	/** Largo del brazo desde el eje hacia afuera. El brazo se estira solo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barredora|Movimiento")
	float Largo = 500.0f;

	/** Grosor del brazo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barredora|Movimiento")
	float GrosorBrazo = 0.15f;

	// --- Golpe ---

	/** Empuje al golpear al jugador. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barredora|Golpe")
	float Fuerza = 1200.0f;

	/** Cuánto lo levanta además de empujarlo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barredora|Golpe")
	float FuerzaVertical = 400.0f;

	/** 0 = lo tira puro de costado (hacia donde va la barra).
	 *  1 = lo tira puro hacia afuera del eje. Intermedio mezcla las dos. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barredora|Golpe", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EmpujeHaciaAfuera = 0.3f;

	/** Segundos mínimos entre dos golpes, para que no lo empuje en loop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barredora|Golpe")
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