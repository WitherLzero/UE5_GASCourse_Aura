// Copyright rynnli


#include "AuraGame/GAS/ExecCalc/ExecCalc_Damage.h"

#include "AuraGame/Stats/AuraStats.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AuraGame/GAS/AuraAbilitySystemLibrary.h"
#include "GameplayMechanics/Core/RPGAbilitySystemLibrary.h"

#include "RPGFramework/GAS/AttributeSets/VitalAttributeSet.h"
#include "AuraGame/GAS/Data/CharacterClassInfo.h"
#include "AuraGame/GAS/AttributeSets/CombatAttributeSet.h"
#include "AuraGame/Types/AuraGameplayTags.h"
#include "RPGFramework/Interaction/CharacterDataInterface.h"
#include "AuraGame/System/AuraGameSetting.h"
#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"
#include "GameplayMechanics/Core/Abilities/RPGPassiveAbility.h"
#include "RPGFramework/Types/RPGGameplayTags.h"


struct RPGDamageStatic
{
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LifeSteal);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ManaSteal);
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);
	
	RPGDamageStatic()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet,Armor,Target,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet,BlockChance,Target,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, CriticalHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, CriticalHitDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, LifeSteal, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, ManaSteal, Source, false);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCombatAttributeSet, PhysicalResistance, Target, false);
	}
};

static const RPGDamageStatic& DamageStatics()
{
	static RPGDamageStatic DStatics;

	// Lazy init: TagsToCaptureDefs must be populated at runtime (not during CDO creation),
	// because FAuraGameplayTags aren't initialized yet at that point.
	if (DStatics.TagsToCaptureDefs.IsEmpty())
	{
		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		if (Tags.Attributes_Resistance_Fire.IsValid())
		{
			DStatics.TagsToCaptureDefs.Reserve(4);
			DStatics.TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, DStatics.FireResistanceDef);
			DStatics.TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, DStatics.LightningResistanceDef);
			DStatics.TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, DStatics.ArcaneResistanceDef);
			DStatics.TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, DStatics.PhysicalResistanceDef);
		}
		// If Tags aren't ready yet, map stays empty — will retry next call.
	}

	return DStatics;
}


UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().LifeStealDef);
	RelevantAttributesToCapture.Add(DamageStatics().ManaStealDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
	
}

void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& Spec, FAggregatorEvaluateParameters EvaluationParameters) const
{
	const FRPGGameplayTags& RPGGameplayTags = FRPGGameplayTags::Get();
	const FAuraGameplayTags& AuraGameplayTags = FAuraGameplayTags::Get();
	
	for (TTuple<FGameplayTag, FGameplayTag> Pair : AuraGameplayTags.DamageTypesToDebuffs)
	{
		const FGameplayTag& DamageType = Pair.Key;
		const FGameplayTag& DebuffType = Pair.Value;
		const FGameplayTag& ResistanceTag = AuraGameplayTags.DamageTypesToResistances[DamageType];
		
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1.f);
		if (TypeDamage > -.5f) // .5 padding for floating point [im]precision
		{
			// Determine if there was a successful debuff
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(RPGGameplayTags.Debuff_Chance, false, -1.f);

			float TargetDebuffResistance = 0.f;
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TagsToCaptureDefs[ResistanceTag], EvaluationParameters, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);
			
			
			const float EffectiveDebuffChance = SourceDebuffChance * ( 100 - TargetDebuffResistance ) / 100.f;
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			if (bDebuff)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

				URPGAbilitySystemLibrary::SetIsSuccessfulDebuff(ContextHandle, true);
				URPGAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);
				URPGAbilitySystemLibrary::SetDebuffType(ContextHandle, DebuffType);

				const float DebuffDamage = Spec.GetSetByCallerMagnitude(RPGGameplayTags.Debuff_Damage, false, -1.f);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(RPGGameplayTags.Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(RPGGameplayTags.Debuff_Frequency, false, -1.f);
				
				URPGAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
				URPGAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
				URPGAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
			}
		}
	}
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	SCOPE_CYCLE_COUNTER(STAT_ExecCalcDamage);
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	
	AActor* SourceAvatar = SourceASC? SourceASC->GetAvatarActor(): nullptr;
	AActor* TargetAvatar = TargetASC? TargetASC->GetAvatarActor(): nullptr;
	ICharacterDataInterface* TargetInterface = Cast<ICharacterDataInterface>(TargetAvatar);
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle EffectContext = Spec.GetContext();
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	
	DetermineDebuff(ExecutionParams, Spec, EvaluationParameters);
	
	// Get damage set by caller Magnitude
	float Damage = 0.f;
	for (const auto& Pair : FAuraGameplayTags::Get().DamageTypesToResistances )
	{
		const FGameplayTag ResistanceTag = Pair.Value;
		const FGameplayEffectAttributeCaptureDefinition* CaptureDef = DamageStatics().TagsToCaptureDefs.Find(ResistanceTag);
		checkf(CaptureDef, TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false);
		
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(*CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);
		
		DamageTypeValue *= ( 100.f - Resistance ) / 100.f;
		
		Damage+=DamageTypeValue;
	}
	
	// Capture BlockChance on target, and determine if there was a successful Block
	// If Block. halve the damage
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);
	
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	URPGAbilitySystemLibrary::SetIsBlockedHit(EffectContext,bBlocked);
	Damage = bBlocked ? Damage / 2.f : Damage;
	
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);
	
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);
	
	UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalcCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(TargetInterface->GetCharacterLevel());
	// ArmorPenetration ignores a percentage of the Target's Armor.	
	const float EffectiveArmor = TargetArmor *  ( 100 - SourceArmorPenetration * ArmorPenetrationCoefficient ) / 100.f;
	
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalcCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetInterface->GetCharacterLevel());
	// Armor ignores a percentage of incoming Damage.
	Damage *= ( 100 - EffectiveArmor * EffectiveArmorCoefficient ) / 100.f;
	
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);
	
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);
	
	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);
	
	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalcCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetInterface->GetCharacterLevel());
	
	// Critical Hit Resistance reduces Critical Hit Chance by a certain percentage
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;
	URPGAbilitySystemLibrary::SetIsCriticalHit(EffectContext,bCriticalHit);
	
	// Double damage plus a bonus if critical hit
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;
	
	const bool bHasHaloOfProtection = TargetTags->HasTagExact(FAuraGameplayTags::Get().State_Buff_HaloOfProtection);
	if (bHasHaloOfProtection)
	{
		if (URPGAbilitySystemComponent* TargetRPGASC = Cast<URPGAbilitySystemComponent>(TargetASC))
		{
			if (FGameplayAbilitySpec* HaloSpec = TargetRPGASC->GetSpecFromAbilityTag(FAuraGameplayTags::Get().Abilities_Passive_HaloOfProtection))
			{
				if (URPGPassiveAbility* HaloAbilityCDO = Cast<URPGPassiveAbility>(HaloSpec->Ability))
				{
					if (const FScalableFloat* ReductionCurve = HaloAbilityCDO->AbilityDataMap.Find(FAuraGameplayTags::Get().Abilities_Passive_HaloOfProtection))
					{
						const float HaloReduction = ReductionCurve->GetValueAtLevel(HaloSpec->Level);
						Damage = FMath::Max(0.f, Damage * (100 - HaloReduction) / 100.f);
					}
				}
			}
		}
	}
	
	const FGameplayModifierEvaluatedData EvaluatedData(UVitalAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);

	// Life Steal and Mana Steal
	if (Damage > 0.f)
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const UAuraGameSetting* GameSetting = GetDefault<UAuraGameSetting>();

		// Life Steal
		float SourceLifeSteal = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().LifeStealDef, EvaluationParameters, SourceLifeSteal);
		SourceLifeSteal = FMath::Max<float>(SourceLifeSteal, 0.f);

		if (SourceLifeSteal > 0.f && GameSetting->LifeStealEffectClass)
		{
			const float LifeToSteal = Damage * (SourceLifeSteal / 100.f);
			FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
			ContextHandle.AddSourceObject(SourceAvatar);
			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GameSetting->LifeStealEffectClass, 1.f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(GameplayTags.Event_Recovery_Life, LifeToSteal);
				SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		// Mana Steal
		float SourceManaSteal = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ManaStealDef, EvaluationParameters, SourceManaSteal);
		SourceManaSteal = FMath::Max<float>(SourceManaSteal, 0.f);

		if (SourceManaSteal > 0.f && GameSetting->ManaStealEffectClass)
		{
			const float ManaToSteal = Damage * (SourceManaSteal / 100.f);
			FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
			ContextHandle.AddSourceObject(SourceAvatar);
			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GameSetting->ManaStealEffectClass, 1.f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(GameplayTags.Event_Recovery_Mana, ManaToSteal);
				SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
