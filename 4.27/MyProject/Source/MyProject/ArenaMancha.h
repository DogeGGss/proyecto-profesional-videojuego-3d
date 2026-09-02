#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaMancha.generated.h"

class UInstancedStaticMeshComponent;
class UArrowComponent;

UCLASS()
class MYPROJECT_API AArenaMancha : public AActor
{
	GENERATED_BODY()

public:
	AArenaMancha();

	/** Toda la arena se reconstruye acá cada vez que tocás un parámetro. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Todos los bloques son instancias de una sola malla: barato de dibujar. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	UInstancedStaticMeshComponent* Bloques;

	/** Marcadores de dónde poner los PlayerStart. Simétricos por construcción. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	UArrowComponent* InicioA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	UArrowComponent* InicioB;

	// --- Medidas generales ---

	/** Lado del cuadrado. 2800 = unos 4,5 segundos de punta a punta corriendo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Medidas")
	float LadoArena = 2800.0f;

	/** Espesor de pisos y paredes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Medidas")
	float Espesor = 60.0f;

	/** Alto de los muros perimetrales. Que nadie se caiga para escapar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Medidas")
	float AlturaMuro = 700.0f;

	// --- Bloque central ---

	/** Lado del conjunto central. Es lo que se rodea corriendo en círculos. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Centro")
	float LadoBloqueCentral = 1200.0f;

	/** Alto del bloque central, o sea la altura del segundo nivel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Centro")
	float AlturaBloqueCentral = 400.0f;

	/** Ancho de los dos pasillos que cruzan el centro en cruz. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Centro")
	float AnchoPasillo = 400.0f;

	// --- Rampas ---

	/** Qué tan tendidas son. Más largo = más suave de subir. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Rampas")
	float LargoRampa = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Rampas")
	float AnchoRampa = 320.0f;

protected:
	/** Agrega un bloque con centro, tamaño en unidades reales y rotación. */
	void AgregarCaja(const FVector& Centro, const FVector& Tamano,
		const FRotator& Rotacion = FRotator::ZeroRotator);

	/** Agrega una rampa que va de un punto a otro, calculando sola la inclinación. */
	void AgregarRampa(const FVector& Desde, const FVector& Hasta);

private:
	/** Medidas reales de la malla asignada, para convertir tamaños a escalas. */
	FVector TamanoMallaBase = FVector(100.f, 100.f, 100.f);
};