// Copyright rynnli


#include "AuraGame/UI/WidgetController/SpellMenuWidgetController.h"

#include "AuraGame/Character/Player/AuraPlayerState.h"
#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"
#include "RPGFramework/GAS/Data/AbilityInfo.h"



void USpellMenuWidgetController::BroadcastInitialValues()
{
	OnInitializeStartupAbilities(GetASC());
	
	SpellPointsChanged.Broadcast(GetAuraPS()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetASC()->OnAbilityStatusChanged.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 NewLevel)
		{
			if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
			{
				SelectedAbility.Status = StatusTag;
				bool bEnableSpendPoints = false;
				bool bEnableEquip = false;
				ShouldEnableButtons(CurrentSpellPoints, StatusTag, bEnableSpendPoints, bEnableEquip);
			FText Description;
			FText NextLevelDescription;
			GetASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
			OnSpellGlobeSelected.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
			}
			
			if (AbilityInfo)
			{
				FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				Info.StatusTag = StatusTag;
				OnAbilityInfoGet.Broadcast(Info);
			}
		});
	
	GetASC()->OnAbilityEquipped.AddUObject(this,&ThisClass::HandleAbilityEquipped);
	
	GetAuraPS()->OnSpellPointsChanged.AddLambda(
		[this](int32 NewPoints)
		{
			SpellPointsChanged.Broadcast(NewPoints);
			CurrentSpellPoints = NewPoints;
			
			bool bEnableSpendPoints = false;
			bool bEnableEquip = false;
			ShouldEnableButtons(CurrentSpellPoints, SelectedAbility.Status, bEnableSpendPoints, bEnableEquip);
			FText Description;
			FText NextLevelDescription;
			GetASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
			OnSpellGlobeSelected.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
		});
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		OnWaitingForEquip.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	const int32 SpellPoints = GetAuraPS()->GetSpellPoints();
	FGameplayTag AbilityStatus;
	
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	const FGameplayAbilitySpec* AbilitySpec = GetASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;

	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	}else
	{
		AbilityStatus = GetASC()->GetStatusFromSpec(*AbilitySpec);
	}
	
	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;
	bool bEnableSpendPoints = false;
	bool bEnableEquip = false;
	ShouldEnableButtons(SpellPoints, AbilityStatus, bEnableSpendPoints, bEnableEquip);
	FText Description;
	FText NextLevelDescription;
	GetASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
	OnSpellGlobeSelected.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);	
}

void USpellMenuWidgetController::SpellGlobeDeselected()
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		OnWaitingForEquip.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.Ability = FRPGGameplayTags::Get().Abilities_None;
	SelectedAbility.Status = FRPGGameplayTags::Get().Abilities_Status_Locked;
	
	OnSpellGlobeSelected.Broadcast(false, false, FText(), FText());	
	
}

void USpellMenuWidgetController::SpendPointButtonPressed()
{
	if (GetAuraPS())
	{
		GetAuraPS()->SpendSpellPoint(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	OnWaitingForEquip.Broadcast(SelectedAbilityType);
	bWaitingForEquipSelection = true;
}

void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	// Make sure selected ability won't be equipped on the slot with different AbilityType
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;
	
	GetASC()->ServerEquipAbility(SelectedAbility.Ability,SlotTag);
}

void USpellMenuWidgetController::ShouldEnableButtons(const int32 SpellPoints, const FGameplayTag& AbilityStatus,
                                                     bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();

	bShouldEnableSpellPointsButton = false;
	bShouldEnableEquipButton = false;
	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
}

void USpellMenuWidgetController::HandleAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag,
	const FGameplayTag& NewSlot, const FGameplayTag& PreviousSlot)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
	
	FRPGAbilityInfo LastSlotInfo;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	OnAbilityInfoGet.Broadcast(LastSlotInfo);
	
	FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = StatusTag;
	Info.InputTag = NewSlot;
	OnAbilityInfoGet.Broadcast(Info);
	
	OnWaitingForEquip.Broadcast(Info.AbilityType);
	bWaitingForEquipSelection = false;
	
	OnSpellGlobeReassigned.Broadcast(AbilityTag);
}
