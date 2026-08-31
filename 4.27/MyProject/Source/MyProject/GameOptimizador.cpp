#include "GameOptimizador.h"
#include "Engine/Engine.h"

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

	// Nos aseguramos de que el puntero al Engine sea válido
	if (GEngine && GetWorld())
	{
		// 1. Desactivar reflejos y brillos especulares en superficies/piso
		GEngine->Exec(GetWorld(), TEXT("r.Specular 0"));               // Elimina el brillo de espejo en las texturas
		GEngine->Exec(GetWorld(), TEXT("r.Reflections.Quality 0"));    // Apaga reflexiones
		GEngine->Exec(GetWorld(), TEXT("r.SSR.Quality 0"));            // Apaga Screen Space Reflections

		// 2. Controlar la exposición (Fijar la luz para que no encandile)
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 0"));   // Desactiva la adaptación automática de la pupila
		GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptation.LensAttenuation 0"));
		GEngine->Exec(GetWorld(), TEXT("r.Tonemapper.Quality 0"));     // Elimina destellos de bloom y sobreexposición

		// 3. Calidad gráfica y sombras planas
		GEngine->Exec(GetWorld(), TEXT("sg.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("sg.PostProcessQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("sg.TextureQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.MipMapLODBias 2"));

		// 4. Rendimiento de pantalla
		GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 70"));
	}
}

// Called every frame
void AGameOptimizador::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}