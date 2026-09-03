#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiplayerGameMode.generated.h"

UCLASS()
class MYPROJECT_API AMultiplayerGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMultiplayerGameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;

    void IniciarCuentaRegresiva();
    void IniciarRonda();
    void TickTimerRonda();
    void OtorgarPunto(class ATagPlayerState* JugadorGanador);

protected:
    FTimerHandle TimerHandle_Reinicio;
    void ReiniciarPartida();

private:
    FTimerHandle TimerHandle_Ronda;
    int32 JugadoresConectados;
};