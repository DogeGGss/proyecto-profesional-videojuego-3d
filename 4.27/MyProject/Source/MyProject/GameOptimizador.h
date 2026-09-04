#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameOptimizador.generated.h"

UCLASS()
class AGameOptimizador : public AActor
{
	GENERATED_BODY()

public:
	// Establece valores predeterminados para las propiedades de este actor
	AGameOptimizador();

	// Función expuesta para que los botones del Widget puedan cambiar la calidad
	UFUNCTION(BlueprintCallable, Category = "Optimizacion")
	void CambiarCalidadGrafica(int32 Nivel);

protected:
	// Se ejecuta cuando el actor aparece en el nivel / inicia la partida
	virtual void BeginPlay() override;

public:
	// Se ejecuta en cada fotograma (no lo usaremos por ahora)
	virtual void Tick(float DeltaTime) override;
};