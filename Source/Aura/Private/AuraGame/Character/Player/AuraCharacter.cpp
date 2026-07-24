// Copyright rynnli


#include "AuraGame/Character/Player/AuraCharacter.h"

#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AuraGame/Stats/AuraStats.h"
#include "AuraGame/Types/AuraGameplayTags.h"
#include "RPGFramework/Types/RPGGameplayTags.h"
#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"
#include "RPGFramework/Player/RPGPlayerController.h"
#include "RPGFramework/Player/RPGPlayerState.h"
#include "RPGFramework/Input/RPGInputConfig.h"
#include "AuraGame/Character/Player/AuraPlayerState.h"
#include "AuraGame/UI/HUD/AuraHUD.h"
#include "AuraGame/GAS/Data/AuraLevelConfig.h"
#include "Components/SplineComponent.h"


AAuraCharacter::AAuraCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
	
	CharacterClassTag = FAuraGameplayTags::Get().Character_Class_Elementalist;
}

void AAuraCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AAuraCharacter::InitDefaultAttributes() const
{
	for (const TSubclassOf<UGameplayEffect> EffectClass : DefaultAttributes)
	{
		ApplyEffectToSelf(EffectClass,1.f);
	}
}

void AAuraCharacter::AddCharacterAbilities()
{
	if (!HasAuthority()) return;
	
	CastChecked<URPGAbilitySystemComponent>(AbilitySystemComponent)->AddCharacterAbilities(CharacterAbilities);
	CastChecked<URPGAbilitySystemComponent>(AbilitySystemComponent)->AddCharacterPassiveAbilities(StartupPassiveAbilities);
	CastChecked<URPGAbilitySystemComponent>(AbilitySystemComponent)->UpdateAbilityStatuses(GetCharacterLevel());
}

void AAuraCharacter::AddToXP_Implementation(int32 InXP)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	
	const int32 CurrentLevel = AuraPlayerState->GetPlayerLevel();
	const int32 CurrentXP = AuraPlayerState->GetXP();
	const int32 NewLevel = AuraPlayerState->LevelConfig->FindLevelForXP(CurrentXP+InXP);
	const int32 NumLevelUps = NewLevel - CurrentLevel;
	
	if (NumLevelUps > 0)
	{
		int32 AttributePointsReward = 0;
		int32 SpellPointsReward = 0;
		
		for (int32 i = 0; i < NumLevelUps; ++i)
		{
			AttributePointsReward += Cast<UAuraLevelConfig>(AuraPlayerState->LevelConfig)->GetAttributePointsReward(CurrentLevel + i);
			SpellPointsReward += Cast<UAuraLevelConfig>(AuraPlayerState->LevelConfig)->GetSpellPointsReward(CurrentLevel + i);
		}
		
		AuraPlayerState->AddToAttributePoints(AttributePointsReward);
		AuraPlayerState->AddToSpellPoints(SpellPointsReward);
		
		AuraPlayerState->AddToLevel(NumLevelUps);
		AuraPlayerState->LevelUp();
		MulticastLevelUpEffects();
	}
	AuraPlayerState->AddToXP(InXP);
	
	
}

void AAuraCharacter::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_AuraCharacterTick);
	Super::Tick(DeltaTime);
	AutoRun();

}

int32 AAuraCharacter::GetCharacterLevel() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

bool AAuraCharacter::HandleNativeInput(FGameplayTag Tag, ERPGInputEvent EventType, FInputActionValue Value)
{
	return OnNativeInput(Tag,EventType,Value);
}


bool AAuraCharacter::OnNativeInput_Implementation(FGameplayTag Tag, ERPGInputEvent EventType, FInputActionValue Value)
{
	SCOPE_CYCLE_COUNTER(STAT_OnNativeInput);
	// ====== 输入消费保护：拦截被消费的 Pressed 后续同帧/跨帧 Held ======
	if (ConsumedInputTag.IsValid())
	{
		if (Tag == ConsumedInputTag)
		{
			if (EventType == ERPGInputEvent::IE_Held)
			{
				return true; // 持续拦截直到按键释放
			}
			if (EventType == ERPGInputEvent::IE_Released)
			{
				ConsumedInputTag = FGameplayTag(); // 释放后解除
			}
		}
		else
		{
			ConsumedInputTag = FGameplayTag(); // 其他按键按下 → 安全解除
		}
	}
	
	// ====== 核心拦截层：检查瞄准微状态 ======
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FRPGGameplayTags::Get().State_Action_Targeting))
	{
		// 拦截左键作为 Confirm
		if (Tag == FRPGGameplayTags::Get().Inputs_LMB && EventType == ERPGInputEvent::IE_Pressed)
		{
			AbilitySystemComponent->LocalInputConfirm();
			ConsumedInputTag = Tag;
			return true; 
		}
		
		// 拦截右键作为 Cancel
		if (Tag == FRPGGameplayTags::Get().Inputs_RMB && EventType == ERPGInputEvent::IE_Pressed)
		{
			AbilitySystemComponent->LocalInputCancel();
			ConsumedInputTag = Tag;
			return true; 
		}
	}

	// ====== 常规按键处理层 ======
	if (Tag == FRPGGameplayTags::Get().Inputs_LMB)
	{
		if (EventType == ERPGInputEvent::IE_Pressed)
		{
			bTargeting = GetPC()->HitEnemyActor();
			bAutoRunning = false;
			
			return true;
		}
		
		if (EventType == ERPGInputEvent::IE_Held)
		{
			if (HoldToMove()) return true;
		}
		
		if (EventType == ERPGInputEvent::IE_Released)
		{
			if (SetupNavPoints()) return true;
		}
	}
	
	return false;
}


// Init ability actor info for the Server
void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

// Init ability actor info for the Client
void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState,this);
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->OnAbilityActorInfoSet();
	InitComponentsWithASC(AbilitySystemComponent);
	InitDefaultAttributes();
	if (ARPGPlayerController* AuraPlayerController = Cast<ARPGPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AbilitySystemComponent,AuraPlayerController,AuraPlayerState);
		}
	}
}

ARPGPlayerController* AAuraCharacter::GetPC()
{
	if (PlayerController == nullptr)
	{
		PlayerController = Cast<ARPGPlayerController>(GetController());
	}
	return PlayerController;
}

void AAuraCharacter::MulticastLevelUpEffects_Implementation() const
{
	BP_OnLevelUpVisualsTriggered();
}

void AAuraCharacter::Move(const FVector2D& InputAxis)
{
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Forward,InputAxis.Y);
	AddMovementInput(Right,InputAxis.X);
}

bool AAuraCharacter::HoldToMove()
{
	SCOPE_CYCLE_COUNTER(STAT_HoldToMove);
	if (!bTargeting)
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		FHitResult Hit;
		if (GetPC()->GetCursorHit(Hit))
		{
			CachedDestination = Hit.ImpactPoint;
		}
				
		const FVector WorldDirection = (CachedDestination - GetActorLocation()).GetSafeNormal();
		AddMovementInput(WorldDirection);
		return true;
	}
	return false;
}

bool AAuraCharacter::SetupNavPoints()
{
	SCOPE_CYCLE_COUNTER(STAT_SetupNavPoints);
	if (!bTargeting)
	{
		if (FollowTime <= ShortPressThreshold)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
				this,GetActorLocation(),CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc: NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc,ESplineCoordinateSpace::World);
				}
				if (NavPath->PathPoints.Num() > 0)
				{
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}
		}
		FollowTime = 0.f;
		bTargeting = false;
		return true;
	}
	return false;
}

void AAuraCharacter::AutoRun()
{
	SCOPE_CYCLE_COUNTER(STAT_AutoRun);
	if (bAutoRunning)
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		AddMovementInput(Direction);
		
		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

