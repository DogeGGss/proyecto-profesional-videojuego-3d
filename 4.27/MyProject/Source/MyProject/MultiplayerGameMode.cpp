#include "MultiplayerGameMode.h"
#include "MultiplayerGameState.h"
#include "TagPlayerState.h"
#include "TimerManager.h"

AMultiplayerGameMode::AMultiplayerGameMode()
{
    GameStateClass = AMultiplayerGameState::StaticClass();
    PlayerStateClass = ATagPlayerState::StaticClass();
    JugadoresConectados = 0;
}

void AMultiplayerGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    JugadoresConectados++;

    if (JugadoresConectados == 2)
    {
        IniciarCuentaRegresiva();
    }
}

void AMultiplayerGameMode::IniciarRonda()
{
    AMultiplayerGameState* GS = GetGameState<AMultiplayerGameState>();
    if (!GS) return;

    GS->EstadoActual = EEstadoJuego::EnRonda;
    GS->TiempoRonda = 30; // 30 segundos límite para atrapar[cite: 1]

    // Leemos directamente la lista oficial de jugadores del servidor
    if (GS->PlayerArray.Num() >= 2)
    {
        ATagPlayerState* PS1 = Cast<ATagPlayerState>(GS->PlayerArray[0]);
        ATagPlayerState* PS2 = Cast<ATagPlayerState>(GS->PlayerArray[1]);

        if (PS1 && PS2)
        {
            // Asignación al azar justo en el mismo momento en que arranca el reloj[cite: 1]
            bool bDado = FMath::RandBool();
            PS1->bEsMancha = bDado;
            PS2->bEsMancha = !bDado;
        }
    }

    GetWorldTimerManager().SetTimer(TimerHandle_Ronda, this, &AMultiplayerGameMode::TickTimerRonda, 1.0f, true);
}

void AMultiplayerGameMode::TickTimerRonda()
{
    AMultiplayerGameState* GS = GetGameState<AMultiplayerGameState>();
    if (!GS) return;

    GS->TiempoRonda--;

    if (GS->TiempoRonda <= 0)
    {
        GetWorldTimerManager().ClearTimer(TimerHandle_Ronda);

        if (GS->EstadoActual == EEstadoJuego::CuentaRegresiva || GS->EstadoActual == EEstadoJuego::FinRonda)
        {
            IniciarRonda(); // Terminó la pausa, arranca la acción
        }
        else if (GS->EstadoActual == EEstadoJuego::EnRonda)
        {
            // El Mancha no lo alcanzó en 30s. El Evasor gana el punto[cite: 1]
            if (GameState->PlayerArray.Num() >= 2)
            {
                ATagPlayerState* PS1 = Cast<ATagPlayerState>(GameState->PlayerArray[0]);
                ATagPlayerState* PS2 = Cast<ATagPlayerState>(GameState->PlayerArray[1]);

                if (PS1 && !PS1->bEsMancha) OtorgarPunto(PS1);
                else if (PS2 && !PS2->bEsMancha) OtorgarPunto(PS2);
            }
        }
    }
}

void AMultiplayerGameMode::IniciarCuentaRegresiva()
{
    AMultiplayerGameState* GS = GetGameState<AMultiplayerGameState>();
    if (GS)
    {
        GS->EstadoActual = EEstadoJuego::CuentaRegresiva;
        GS->TiempoRonda = 5; // 5 segundos de cuenta regresiva
        GetWorldTimerManager().SetTimer(TimerHandle_Ronda, this, &AMultiplayerGameMode::TickTimerRonda, 1.0f, true);
    }
}

void AMultiplayerGameMode::OtorgarPunto(ATagPlayerState* JugadorGanador)
{
    AMultiplayerGameState* GS = GetGameState<AMultiplayerGameState>();
    if (!GS || !JugadorGanador) return;

    // CANDADO 1: Si por un microsegundo de lag llega un doble toque, y ya no estamos EnRonda, lo ignoramos.
    if (GS->EstadoActual != EEstadoJuego::EnRonda) return;

    // Sumar el punto
    if (GameState->PlayerArray.Num() >= 2)
    {
        if (JugadorGanador == GameState->PlayerArray[0]) GS->PuntosJugador1++;
        else GS->PuntosJugador2++;

        // CANDADO 2: Revocamos los roles inmediatamente.
        // Al dejar de ser Mancha, es físicamente imposible que un jugador sume puntos durante los 5s de pausa.
        ATagPlayerState* PS1 = Cast<ATagPlayerState>(GameState->PlayerArray[0]);
        ATagPlayerState* PS2 = Cast<ATagPlayerState>(GameState->PlayerArray[1]);
        if (PS1) PS1->bEsMancha = false;
        if (PS2) PS2->bEsMancha = false;
    }

    // Comprobar condición de victoria (al mejor de 5)
    if (GS->PuntosJugador1 >= 3 || GS->PuntosJugador2 >= 3)
    {
        GS->EstadoActual = EEstadoJuego::FinPartida;
        GetWorldTimerManager().ClearTimer(TimerHandle_Ronda);
    }
    else
    {
        // Nadie llegó a 3, configurar pausa de 5 segundos
        GS->EstadoActual = EEstadoJuego::FinRonda;
        GS->TiempoRonda = 5;
        GetWorldTimerManager().SetTimer(TimerHandle_Ronda, this, &AMultiplayerGameMode::TickTimerRonda, 1.0f, true);
    }
}