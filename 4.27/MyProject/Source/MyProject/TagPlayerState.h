#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TagPlayerState.generated.h"

UCLASS()
class MYPROJECT_API ATagPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Rol")
        bool bEsMancha;
};