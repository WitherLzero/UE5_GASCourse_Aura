// Copyright rynnli


#include "AuraGame/GAS/Abilities/AuraFireBolt.h"

#include "AuraGame/Types/AuraGameplayTags.h"

FText UAuraFireBolt::GetDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Fire);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	FNumberFormattingOptions FloatOpts;
	FloatOpts.MinimumFractionalDigits = 1;
	FloatOpts.MaximumFractionalDigits = 1;

	if (Level == 1)
	{
		return FText::Format(INVTEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Small>Level: </><Level>{0}</>\n"
			"<Small>ManaCost: </><ManaCost>{1}</>\n"
			"<Small>Cooldown: </><Cooldown>{2}</>\n\n"
			"<Default>Launches a bolt of fire, "
			"exploding on impact and dealing: </>"
			"<Damage>{3}</><Default> fire damage with"
			" a chance to burn</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(Damage));
	}
	else
	{
		return FText::Format(INVTEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Small>Level: </><Level>{0}</>\n"
			"<Small>ManaCost: </><ManaCost>{1}</>\n"
			"<Small>Cooldown: </><Cooldown>{2}</>\n\n"
			"<Default>Launches {3} bolts of fire, "
			"exploding on impact and dealing: </>"
			"<Damage>{4}</><Default> fire damage with"
			" a chance to burn</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(FMath::Min(Level, NumProjectiles)),
			FText::AsNumber(Damage));
	}
}

FText UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Fire);
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
			"<Default>Launches {3} bolts of fire, "
			"exploding on impact and dealing: </>"
			"<Damage>{4}</><Default> fire damage with"
			" a chance to burn</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(FMath::Min(Level, NumProjectiles)),
			FText::AsNumber(Damage));
}
