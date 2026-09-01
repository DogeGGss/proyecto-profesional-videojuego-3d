#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerGameState.generated.h"

UENUM(BlueprintType)
enum class EEstadoJuego : uint8 { Esperando, CuentaRegresiva, EnRonda, FinRonda, FinPartida };

UCLASS()
class MYPROJECT_API AMultiplayerGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AMultiplayerGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Partida")
        EEstadoJuego EstadoActual;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Partida")
        int32 TiempoRonda;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Partida")
        int32 PuntosJugador1;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Partida")
        int32 PuntosJugador2;
};