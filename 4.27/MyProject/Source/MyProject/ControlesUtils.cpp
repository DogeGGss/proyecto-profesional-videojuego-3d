#include "ControlesUtils.h"
#include "GameFramework/InputSettings.h"

bool UControlesUtils::ReasignarTecla(FName NombreDeAccion, FKey NuevaTecla)
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (!Settings) return false;

	TArray<FInputActionKeyMapping> Mappings;
	Settings->GetActionMappingByName(NombreDeAccion, Mappings);

	if (Mappings.Num() > 0)
	{
		// Borramos la asignación vieja
		for (FInputActionKeyMapping& Mapa : Mappings)
		{
			Settings->RemoveActionMapping(Mapa);
		}

		// Asignamos la nueva y guardamos
		FInputActionKeyMapping NuevoMapa = Mappings[0];
		NuevoMapa.Key = NuevaTecla;
		Settings->AddActionMapping(NuevoMapa);

		// Esto guarda el archivo Input.ini en el disco duro automáticamente
		Settings->SaveKeyMappings();
		Settings->ForceRebuildKeymaps();
		return true;
	}
	return false;
}

bool UControlesUtils::ReasignarEje(FName NombreDeEje, FKey NuevaTecla, float Escala)
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (!Settings) return false;

	TArray<FInputAxisKeyMapping> Mappings;
	Settings->GetAxisMappingByName(NombreDeEje, Mappings);
	bool bEncontrado = false;

	for (FInputAxisKeyMapping& Mapa : Mappings)
	{
		// Buscar la escala correcta y ASEGURARSE de que no es un botón de joystick
		if (FMath::IsNearlyEqual(Mapa.Scale, Escala) && !Mapa.Key.IsGamepadKey())
		{
			Settings->RemoveAxisMapping(Mapa);

			FInputAxisKeyMapping NuevoMapa = Mapa;
			NuevoMapa.Key = NuevaTecla;
			Settings->AddAxisMapping(NuevoMapa);

			bEncontrado = true;
			break;
		}
	}

	if (bEncontrado)
	{
		Settings->SaveKeyMappings();
		Settings->ForceRebuildKeymaps();
		return true;
	}
	return false;
}

bool UControlesUtils::TeclaYaEstaEnUso(FKey TeclaAProbar, FName& AccionEnConflicto)
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (!Settings) return false;

	// 1. Revisar Acciones (Saltar, etc)
	TArray<FName> NombresAcciones;
	Settings->GetActionNames(NombresAcciones);
	for (FName Accion : NombresAcciones)
	{
		TArray<FInputActionKeyMapping> Mappings;
		Settings->GetActionMappingByName(Accion, Mappings);
		for (const FInputActionKeyMapping& Mapa : Mappings)
		{
			if (Mapa.Key == TeclaAProbar)
			{
				AccionEnConflicto = Accion;
				return true;
			}
		}
	}

	// 2. Revisar Ejes (MoverAdelante, MoverseLado, etc)
	TArray<FName> NombresEjes;
	Settings->GetAxisNames(NombresEjes);
	for (FName Eje : NombresEjes)
	{
		TArray<FInputAxisKeyMapping> Mappings;
		Settings->GetAxisMappingByName(Eje, Mappings);
		for (const FInputAxisKeyMapping& Mapa : Mappings)
		{
			if (Mapa.Key == TeclaAProbar)
			{
				AccionEnConflicto = Eje;
				return true;
			}
		}
	}

	return false;
}

void UControlesUtils::RestaurarTeclasPorDefecto()
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (Settings)
	{
		// Recarga la configuración limpia de fábrica
		Settings->ReloadConfig(Settings->GetClass(), *GInputIni, LOAD_None);
		Settings->ForceRebuildKeymaps();
	}
}

FKey UControlesUtils::ObtenerTeclaDeEje(FName NombreDeEje, float Escala)
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (Settings)
	{
		TArray<FInputAxisKeyMapping> Mappings;
		Settings->GetAxisMappingByName(NombreDeEje, Mappings);
		for (const FInputAxisKeyMapping& Mapa : Mappings)
		{
			if (FMath::IsNearlyEqual(Mapa.Scale, Escala) && !Mapa.Key.IsGamepadKey())
			{
				return Mapa.Key;
			}
		}
	}
	return FKey(); // Retorna vacío si no hay nada
}

FKey UControlesUtils::ObtenerTeclaDeAccion(FName NombreDeAccion)
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (Settings)
	{
		TArray<FInputActionKeyMapping> Mappings;
		Settings->GetActionMappingByName(NombreDeAccion, Mappings);
		for (const FInputActionKeyMapping& Mapa : Mappings)
		{
			if (!Mapa.Key.IsGamepadKey())
			{
				return Mapa.Key;
			}
		}
	}
	return FKey();
}