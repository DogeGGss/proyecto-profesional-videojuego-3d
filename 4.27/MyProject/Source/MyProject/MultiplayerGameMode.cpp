#include "MultiplayerGameMode.h"
#include "MultiplayerGameState.h"
#include "TagPlayerState.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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

    // Descongelamos las físicas para que empiece la persecución
    if (GS->PlayerArray.Num() >= 2)
    {
        for (int i = 0; i < 2; i++)
        {
            APawn* Peon = GS->PlayerArray[i]->GetPawn();
            if (Peon)
            {
                ACharacter* Personaje = Cast<ACharacter>(Peon);
                if (Personaje && Personaje->GetCharacterMovement())
                {
                    Personaje->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                }
            }
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

        if (GS->EstadoActual == EEstadoJuego::CuentaRegresiva)
        {
            IniciarRonda(); // Terminan los 5s de espera, arranca la partida
        }
        else if (GS->EstadoActual == EEstadoJuego::EnRonda)
        {
            // El Mancha no lo alcanzó en 30s. El Evasor gana el punto
            if (GameState->PlayerArray.Num() >= 2)
            {
                ATagPlayerState* PS1 = Cast<ATagPlayerState>(GameState->PlayerArray[0]);
                ATagPlayerState* PS2 = Cast<ATagPlayerState>(GameState->PlayerArray[1]);

                if (PS1 && !PS1->bEsMancha) OtorgarPunto(PS1);
                else if (PS2 && !PS2->bEsMancha) OtorgarPunto(PS2);
            }
        }
        else if (GS->EstadoActual == EEstadoJuego::FinRonda)
        {
            // Terminan los 5s del marcador. Iniciamos la cuenta regresiva (que ahora se encarga de teletransportar)
            IniciarCuentaRegresiva();
        }
    }
}

void AMultiplayerGameMode::IniciarCuentaRegresiva()
{
    AMultiplayerGameState* GS = GetGameState<AMultiplayerGameState>();
    if (!GS) return;

    GS->EstadoActual = EEstadoJuego::CuentaRegresiva;
    GS->TiempoRonda = 5;

    // Buscamos los Spawns en el mapa
    TArray<AActor*> PuntosDeSpawn;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PuntosDeSpawn);

    if (PuntosDeSpawn.Num() >= 2 && GS->PlayerArray.Num() >= 2)
    {
        for (int i = 0; i < 2; i++)
        {
            APawn* Peon = GS->PlayerArray[i]->GetPawn();
            if (Peon)
            {
                // 1. Teletransportamos el cuerpo
                Peon->SetActorLocation(PuntosDeSpawn[i]->GetActorLocation());

                // 2. Forzamos al controlador (la cámara/cabeza) a mirar hacia donde indica el Player Start
                APlayerController* PC = Cast<APlayerController>(Peon->GetController());
                if (PC)
                {
                    // Reemplazamos SetControlRotation (que solo afecta al servidor y el cliente sobreescribe)
                    // por ClientSetRotation, que envía una orden RPC (Remote Procedure Call) directa al cliente.
                    PC->ClientSetRotation(PuntosDeSpawn[i]->GetActorRotation(), true);
                }

                // 3. Congelamos las piernas
                ACharacter* Personaje = Cast<ACharacter>(Peon);
                if (Personaje && Personaje->GetCharacterMovement())
                {
                    Personaje->GetCharacterMovement()->SetMovementMode(MOVE_None);
                }
            }
        }
    }

    GetWorldTimerManager().SetTimer(TimerHandle_Ronda, this, &AMultiplayerGameMode::TickTimerRonda, 1.0f, true);
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

        // NUEVO: Esperamos 5 segundos para que lean el cartel de VICTORIA/DERROTA y reiniciamos
        GetWorldTimerManager().SetTimer(TimerHandle_Reinicio, this, &AMultiplayerGameMode::ReiniciarPartida, 5.0f, false);
    }
    else
    {
        // Nadie llegó a 3, configurar pausa de 5 segundos
        GS->EstadoActual = EEstadoJuego::FinRonda;
        GS->TiempoRonda = 5;
        GetWorldTimerManager().SetTimer(TimerHandle_Ronda, this, &AMultiplayerGameMode::TickTimerRonda, 1.0f, true);
    }
}

FString AMultiplayerGameState::ObtenerResultadoPartida(APlayerController* JugadorLocal)
{
    // 1. Si la partida sigue su curso, la pantalla queda transparente
    if (EstadoActual != EEstadoJuego::FinPartida)
    {
        return TEXT("");
    }

    // 2. Seguridad anti-crasheos
    if (!JugadorLocal) return TEXT("");

    // 3. ¿Quién está mirando la pantalla? (HasAuthority confirma si es el Host)
    bool bEsHost = JugadorLocal->HasAuthority();

    if (bEsHost)
    {
        // Somos el Jugador 1
        return (PuntosJugador1 >= 3) ? TEXT("¡VICTORIA!") : TEXT("DERROTA");
    }
    else
    {
        // Somos el Jugador 2 (Cliente)
        return (PuntosJugador2 >= 3) ? TEXT("¡VICTORIA!") : TEXT("DERROTA");
    }
}

void AMultiplayerGameMode::ReiniciarPartida()
{
    // "?Restart" le indica a Unreal que recargue exactamente el mismo mapa actual.
    // El 'false' significa que las conexiones no se caen, simplemente se recargan.
    GetWorld()->ServerTravel("?Restart", false);
}