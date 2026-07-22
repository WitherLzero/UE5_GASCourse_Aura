// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "AuraGame/Character/AuraCharacterBase.h"
#include "RPGFramework/Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"


class UActionComponent;
class UCombatComponent;
class ARPGPlayerController;
class USplineComponent;

UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
protected:
	virtual void BeginPlay() override;
	
	virtual void InitDefaultAttributes() const override;
	virtual void AddCharacterAbilities() override;
	
	/* Player Interface */
	virtual void AddToXP_Implementation(int32 InXP) override;
	/* end Player Interface */
	
	/* Combat Interface */
	virtual int32 GetCharacterLevel() const override;
	/* end Combat Interface */
	
	/* Input Interactable */
	virtual bool HandleNativeInput(FGameplayTag Tag, ERPGInputEvent EventType, FInputActionValue Value) override;
	/* end Input Interactable*/
	
	// Input handlers
	virtual bool OnNativeInput_Implementation(FGameplayTag Tag, ERPGInputEvent EventType, FInputActionValue Value) override;
	virtual void Move(const FVector2D& InputAxis) override;
	bool HoldToMove();
	bool SetupNavPoints();
	void AutoRun();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpEffects() const;
	
	UFUNCTION(BlueprintImplementableEvent, Category=" RPG | Effects")
	void BP_OnLevelUpVisualsTriggered() const;
	
	UPROPERTY()
	TObjectPtr<ARPGPlayerController> PlayerController;
	

	
private:
	virtual void InitAbilityActorInfo() override;
	ARPGPlayerController* GetPC();
	

	/*
	 *  Click-to-Move vars
	 */
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;
	bool bTargeting = false;
	
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	/** 上一次被 OnNativeInput 消费掉（return true）的 Pressed 事件对应的 InputTag */
	FGameplayTag ConsumedInputTag;
};
