// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "AuraWidgetController.h"
#include "GameplayTagContainer.h"
#include "RPGFramework/Types/RPGGameplayTags.h"
#include "SpellMenuWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled, FText, Description, FText, NextLevelDescription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaitingForEquipSelectionSignature, const FGameplayTag&, AbilityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpellGlobeReassignedSignature, const FGameplayTag&, AbilityTag);
/**
 * 
 */

struct FSelectedAbility
{
	FGameplayTag Ability = FGameplayTag();
	FGameplayTag Status = FGameplayTag();
};

UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;
	
	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);
	
	UFUNCTION(BlueprintCallable)
	void SpellGlobeDeselected();
	
	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();
	
	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();	
	
	UFUNCTION(BlueprintCallable)
	void SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType);	
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Spells")
	FOnPlayerInfoChangedSignature SpellPointsChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnSpellGlobeSelectedSignature OnSpellGlobeSelected;
	
	UPROPERTY(BlueprintAssignable)
	FOnSpellGlobeReassignedSignature OnSpellGlobeReassigned;	
	
	UPROPERTY(BlueprintAssignable)
	FOnWaitingForEquipSelectionSignature OnWaitingForEquip;
	
	UPROPERTY(BlueprintReadOnly)
	bool bWaitingForEquipSelection = false;
private:
	void ShouldEnableButtons(const int32 SpellPoints,const FGameplayTag& AbilityStatus, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton);
	void HandleAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& NewSlot, const FGameplayTag& PreviousSlot);
	
	
	FSelectedAbility SelectedAbility = { FRPGGameplayTags::Get().Abilities_None, FRPGGameplayTags::Get().Abilities_Status_Locked};
	int32 CurrentSpellPoints = 0;
	

};
