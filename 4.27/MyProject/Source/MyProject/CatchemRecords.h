// Catch'em - envio del tiempo ganador a la tabla de records.
// Libreria de funciones para Blueprint: no hay que instanciar nada,
// el nodo aparece solo en cualquier Blueprint.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CatchemRecords.generated.h"

UCLASS()
class UCatchemRecords : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Manda un tiempo a la planilla de records. No bloquea el juego:
	 * dispara la peticion y sigue. Si no hay internet, no pasa nada,
	 * queda un aviso en el log.
	 *
	 * @param Jugador   Nombre a mostrar. Si va vacio, el servidor guarda "Anonimo".
	 * @param Segundos  El numero crudo del cronometro, con decimales.
	 */
	UFUNCTION(BlueprintCallable, Category = "Catch'em|Records", meta = (DisplayName = "Enviar Tiempo a Records"))
	static void EnviarTiempo(const FString& Jugador, float Segundos);
};
