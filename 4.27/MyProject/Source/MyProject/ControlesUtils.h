#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ControlesUtils.generated.h"

UCLASS()
class MYPROJECT_API UControlesUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Cambia la tecla y guarda en disco automáticamente
	UFUNCTION(BlueprintCallable, Category = "Controles|Teclas")
		static bool ReasignarTecla(FName NombreDeAccion, FKey NuevaTecla);

	// Verifica si la tecla ya está asignada a otra cosa (Resolución de conflictos)
	UFUNCTION(BlueprintPure, Category = "Controles|Teclas")
		static bool TeclaYaEstaEnUso(FKey TeclaAProbar, FName& AccionEnConflicto);

	// Restaura las teclas originales definidas en el editor
	UFUNCTION(BlueprintCallable, Category = "Controles|Teclas")
		static void RestaurarTeclasPorDefecto();

	// Cambia la tecla de un Eje (Movimiento) respetando su escala (1.0 o -1.0)
	UFUNCTION(BlueprintCallable, Category = "Controles|Teclas")
		static bool ReasignarEje(FName NombreDeEje, FKey NuevaTecla, float Escala);

	// Lee la tecla actual de un eje ignorando el joystick
	UFUNCTION(BlueprintPure, Category = "Controles|Teclas")
		static FKey ObtenerTeclaDeEje(FName NombreDeEje, float Escala);

	// Lee la tecla actual de una acción ignorando el joystick
	UFUNCTION(BlueprintPure, Category = "Controles|Teclas")
		static FKey ObtenerTeclaDeAccion(FName NombreDeAccion);
};