// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "RPGFramework/GAS/Abilities/DamageGameplayAbility.h"
#include "AuraArcaneShards.generated.h"

/**
 * Arcane Shards - Radial arcane damage ability
 */
UCLASS()
class AURA_API UAuraArcaneShards : public UDamageGameplayAbility
{
	GENERATED_BODY()
public:
	virtual FText GetDescription(int32 Level) override;
	virtual FText GetNextLevelDescription(int32 Level) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumShards = 4;
};
