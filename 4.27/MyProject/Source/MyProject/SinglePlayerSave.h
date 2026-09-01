#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SinglePlayerSave.generated.h"

UCLASS()
class MYPROJECT_API USinglePlayerSave : public USaveGame
{
    GENERATED_BODY()

public:
    USinglePlayerSave();

    UPROPERTY(VisibleAnywhere, Category = "Tiempos")
        float MejorTiempo;
};