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
	// Nos aseguramos de que el puntero al Engine sea válido
	if (!GEngine || !GetWorld()) return;

	UGameUserSettings* Ajustes = UGameUserSettings::GetGameUserSettings();

	if (Nivel == 0) // BAJO (Tus optimizaciones extremas originales)
	{
		// 1. Desactivar reflejos y brillos especulares
		GEngine->Exec(GetWorld(), TEXT("r.Specular 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 0"));

		// 2. Controlar la exposición
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 0"));

		// 3. Calidad gráfica y sombras planas
		GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("sg.PostProcessQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("sg.TextureQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 2"));

		// 4. Rendimiento de pantalla
		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 70"));

		// Le avisamos a Unreal que estamos en calidad mínima
		if (Ajustes) Ajustes->SetOverallScalabilityLevel(0);
	}
	else if (Nivel == 1) // MEDIO (Ajustado para mayor diferencia)
	{
		GEngine->Exec(GetWorld(), TEXT("r.Specular 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 1"));

		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 1"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 1"));

		GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("sg.PostProcessQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("sg.TextureQuality 1"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 1"));

		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 85"));

		if (Ajustes) Ajustes->SetOverallScalabilityLevel(1);
	}
	else if (Nivel >= 2) // ALTO (Calidad máxima)
	{
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

		if (Ajustes) Ajustes->SetOverallScalabilityLevel(3);
	}

	// Guardamos en el archivo .ini para que se mantenga al reiniciar (Criterio de Aceptación)
	if (Ajustes) Ajustes->ApplySettings(false);
}

// Called every frame
void AGameOptimizador::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}