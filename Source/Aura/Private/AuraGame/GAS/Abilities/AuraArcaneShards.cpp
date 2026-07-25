// Copyright rynnli


#include "AuraGame/GAS/Abilities/AuraArcaneShards.h"

#include "AuraGame/Types/AuraGameplayTags.h"

FText UAuraArcaneShards::GetDescription(int32 Level)
{
	const int32 ScaledDamage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Arcane);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	FNumberFormattingOptions FloatOpts;
	FloatOpts.MinimumFractionalDigits = 1;
	FloatOpts.MaximumFractionalDigits = 1;

	if (Level == 1)
	{
		return FText::Format(INVTEXT(
			"<Title>ARCANE SHARDS</>\n\n"
			"<Small>Level: </><Level>{0}</>\n"
			"<Small>ManaCost: </><ManaCost>{1}</>\n"
			"<Small>Cooldown: </><Cooldown>{2}</>\n\n"
			"<Default>Summon a shard of arcane energy, "
			"causing radial arcane damage of </>"
			"<Damage>{3}</><Default> at the shard origin.</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(ScaledDamage));
	}
	else
	{
		return FText::Format(INVTEXT(
			"<Title>ARCANE SHARDS</>\n\n"
			"<Small>Level: </><Level>{0}</>\n"
			"<Small>ManaCost: </><ManaCost>{1}</>\n"
			"<Small>Cooldown: </><Cooldown>{2}</>\n\n"
			"<Default>Summon {3} shards of arcane energy, "
			"causing radial arcane damage of </>"
			"<Damage>{4}</><Default> at the shard origins.</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(FMath::Min(Level, MaxNumShards)),
			FText::AsNumber(ScaledDamage));
	}
}

FText UAuraArcaneShards::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Arcane);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	FNumberFormattingOptions FloatOpts;
	FloatOpts.MinimumFractionalDigits = 1;
	FloatOpts.MaximumFractionalDigits = 1;

	return FText::Format(INVTEXT(
			"<Title>NEXT LEVEL: </>\n\n"
			"<Small>Level: </><Level>{0}</>\n"
			"<Small>ManaCost: </><ManaCost>{1}</>\n"
			"<Small>Cooldown: </><Cooldown>{2}</>\n\n"
			"<Default>Summon {3} shards of arcane energy, "
			"causing radial arcane damage of </>"
			"<Damage>{4}</><Default> at the shard origins.</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(FMath::Min(Level, MaxNumShards)),
			FText::AsNumber(ScaledDamage));
}
