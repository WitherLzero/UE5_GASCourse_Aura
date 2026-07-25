// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameplayMechanics/Core/Abilities/RPGBeamSpell.h"
#include "AuraElectrocute.generated.h"

/**
 * Electrocute - Beam spell dealing lightning damage with shock propagation
 */
UCLASS()
class AURA_API UAuraElectrocute : public URPGBeamSpell
{
	GENERATED_BODY()
public:
	virtual FText GetDescription(int32 Level) override;
	virtual FText GetNextLevelDescription(int32 Level) override;
};
