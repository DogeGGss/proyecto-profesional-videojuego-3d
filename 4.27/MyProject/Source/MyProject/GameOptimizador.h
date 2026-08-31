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

protected:
	// Se ejecuta cuando el actor aparece en el nivel / inicia la partida
	virtual void BeginPlay() override;

public:
	// Se ejecuta en cada fotograma (no lo usaremos por ahora)
	virtual void Tick(float DeltaTime) override;
};