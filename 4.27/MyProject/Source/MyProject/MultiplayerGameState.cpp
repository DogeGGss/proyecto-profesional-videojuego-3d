#include "MultiplayerGameState.h"
#include "Net/UnrealNetwork.h"

AMultiplayerGameState::AMultiplayerGameState()
{
    EstadoActual = EEstadoJuego::Esperando;
    TiempoRonda = 30;
    PuntosJugador1 = 0;
    PuntosJugador2 = 0;
}

// Esta función es obligatoria en C++ para que las variables viajen por la red
void AMultiplayerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMultiplayerGameState, EstadoActual);
    DOREPLIFETIME(AMultiplayerGameState, TiempoRonda);
    DOREPLIFETIME(AMultiplayerGameState, PuntosJugador1);
    DOREPLIFETIME(AMultiplayerGameState, PuntosJugador2);
}

