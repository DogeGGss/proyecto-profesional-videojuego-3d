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
		// Leemos qué calidad dejó guardada el jugador la última vez
		int32 CalidadGuardada = Ajustes->GetOverallScalabilityLevel();

		// Traducimos los 4 niveles de Unreal (0,1,2,3) a tus 3 botones (0=Bajo, 1=Medio, 2=Alto)
		int32 NivelAAplicar = 0;
		if (CalidadGuardada == 2) NivelAAplicar = 1; // Medio
		if (CalidadGuardada >= 3) NivelAAplicar = 2; // Alto

		// Ejecutamos tus comandos automáticamente al iniciar
		CambiarCalidadGrafica(NivelAAplicar);
	}
}

void AGameOptimizador::CambiarCalidadGrafica(int32 Nivel)
{
	if (!GEngine || !GetWorld()) return;

	UGameUserSettings* Ajustes = UGameUserSettings::GetGameUserSettings();

	if (Nivel == 0) // BAJO
	{
		// 1. Primero dejamos que Unreal guarde y aplique su base
		if (Ajustes) {
			Ajustes->SetOverallScalabilityLevel(0);
			Ajustes->ApplySettings(false);
		}

		// 2. DESPUÉS aplicamos tus optimizaciones extremas para que sobreescriban a Unreal
		GEngine->Exec(GetWorld(), TEXT("r.Specular 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 0"));
		GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("sg.PostProcessQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("sg.TextureQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 2"));
		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 70"));
	}
	else if (Nivel == 1) // MEDIO
	{
		if (Ajustes) {
			Ajustes->SetOverallScalabilityLevel(1);
			Ajustes->ApplySettings(false);
		}

		// Valores intermedios reales: Sombras en 2 (Alta) para que no desaparezcan,
		// pero Postprocesado en 1 (Bajo) y resolución al 85% para ganar FPS.
		GEngine->Exec(GetWorld(), TEXT("r.Specular 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 1"));
		GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 2"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 2"));
		GEngine->Exec(GetWorld(), TEXT("sg.PostProcessQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("sg.TextureQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 1")); 
		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 85"));
	}
	else if (Nivel >= 2) // ALTO
	{
		if (Ajustes) {
			Ajustes->SetOverallScalabilityLevel(3); // 3 = Épico en Unreal
			Ajustes->ApplySettings(false);
		}

		GEngine->Exec(GetWorld(), TEXT("r.Specular 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 2"));
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 2"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 3"));
		GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 3"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 3"));
		GEngine->Exec(GetWorld(), TEXT("sg.PostProcessQuality 3"));
		GEngine->Exec(GetWorld(), TEXT("sg.TextureQuality 3"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 100"));
	}
}

// Called every frame
void AGameOptimizador::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}