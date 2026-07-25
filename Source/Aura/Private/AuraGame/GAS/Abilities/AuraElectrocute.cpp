// Copyright rynnli


#include "AuraGame/GAS/Abilities/AuraElectrocute.h"

#include "AuraGame/Types/AuraGameplayTags.h"

FText UAuraElectrocute::GetDescription(int32 Level)
{
	const int32 ScaledDamage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Lightning);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	FNumberFormattingOptions FloatOpts;
	FloatOpts.MinimumFractionalDigits = 1;
	FloatOpts.MaximumFractionalDigits = 1;

	if (Level == 1)
	{
		return FText::Format(INVTEXT(
			"<Title>ELECTROCUTE</>\n\n"
			"<Small>Level: </><Level>{0}</>\n"
			"<Small>ManaCost: </><ManaCost>{1}</>\n"
			"<Small>Cooldown: </><Cooldown>{2}</>\n\n"
			"<Default>Emits a beam of lightning, "
			"connecting with the target, repeatedly causing </>"
			"<Damage>{3}</><Default> lightning damage with"
			" a chance to stun</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(ScaledDamage));
	}
	else
	{
		return FText::Format(INVTEXT(
			"<Title>ELECTROCUTE</>\n\n"
			"<Small>Level: </><Level>{0}</>\n"
			"<Small>ManaCost: </><ManaCost>{1}</>\n"
			"<Small>Cooldown: </><Cooldown>{2}</>\n\n"
			"<Default>Emits a beam of lightning, "
			"propagating to {3} additional targets nearby, causing </>"
			"<Damage>{4}</><Default> lightning damage with"
			" a chance to stun</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(FMath::Min(Level, MaxNumShockTargets - 1)),
			FText::AsNumber(ScaledDamage));
	}
}

FText UAuraElectrocute::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Lightning);
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
			"<Default>Emits a beam of lightning, "
			"propagating to {3} additional targets nearby, causing </>"
			"<Damage>{4}</><Default> lightning damage with"
			" a chance to stun</>"),
			FText::AsNumber(Level),
			FText::AsNumber(ManaCost, &FloatOpts),
			FText::AsNumber(Cooldown, &FloatOpts),
			FText::AsNumber(FMath::Min(Level, MaxNumShockTargets - 1)),
			FText::AsNumber(ScaledDamage));
}
