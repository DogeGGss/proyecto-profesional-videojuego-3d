#include "GameOptimizador.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"

// Sets default values
AGameOptimizador::AGameOptimizador()
{
	// Desactivamos el Tick para no gastar recursos procesando este actor cada frame
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AGameOptimizador::BeginPlay()
{
	Super::BeginPlay();

	UGameUserSettings* Ajustes = UGameUserSettings::GetGameUserSettings();
	if (Ajustes)
	{
		// Leemos la calidad que el menú dejó guardada
		int32 CalidadGuardada = Ajustes->GetOverallScalabilityLevel();

		int32 NivelAAplicar = 0; // Por defecto arranca en Bajo

		// Los números coinciden con los del menú
		if (CalidadGuardada == 1) NivelAAplicar = 1; // Medio
		if (CalidadGuardada >= 2) NivelAAplicar = 2; // Alto

		// Reaplicamos los comandos al cargar el mapa multijugador
		CambiarCalidadGrafica(NivelAAplicar);
	}
}

void AGameOptimizador::CambiarCalidadGrafica(int32 Nivel)
{
	if (!GEngine || !GetWorld()) return;

	UGameUserSettings* Ajustes = UGameUserSettings::GetGameUserSettings();
	if (!Ajustes) return;

	if (Nivel == 0) // BAJO
	{
		// Usamos el API nativa en lugar de Exec para los grupos 'sg.'
		Ajustes->SetOverallScalabilityLevel(0);
		Ajustes->ApplySettings(false);

		// Dejamos los comandos 'r.' (rendering puro) en Exec
		GEngine->Exec(GetWorld(), TEXT("r.Specular 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 2"));
		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 70"));
	}
	else if (Nivel == 1) // MEDIO
	{
		Ajustes->SetOverallScalabilityLevel(1);
		Ajustes->SetShadowQuality(1);
		// Reducimos el suavizado de bordes para que se note más rústico
		Ajustes->SetAntiAliasingQuality(1);
		Ajustes->ApplySettings(false);

		GEngine->Exec(GetWorld(), TEXT("r.Specular 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 1"));

		// Apagamos las sombras de contacto de las esquinas
		GEngine->Exec(GetWorld(), TEXT("r.AmbientOcclusionLevels 0"));

		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 75")); // Bajamos a 75%
	}
	else if (Nivel >= 2) // ALTO
	{
		Ajustes->SetOverallScalabilityLevel(3); // 3 = Épico en Unreal
		Ajustes->ApplySettings(false);

		GEngine->Exec(GetWorld(), TEXT("r.Specular 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 2"));
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 2"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 3"));

		// Restauramos las sombras de contacto para la calidad máxima
		GEngine->Exec(GetWorld(), TEXT("r.AmbientOcclusionLevels 3"));

		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 3"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 100"));
	}
}
// Called every frame
void AGameOptimizador::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}