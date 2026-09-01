#include "TagPlayerState.h"
#include "Net/UnrealNetwork.h"

void ATagPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATagPlayerState, bEsMancha);
}