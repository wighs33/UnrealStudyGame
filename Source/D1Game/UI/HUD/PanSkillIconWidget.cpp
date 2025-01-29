﻿#include "PanSkillIconWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanSkillIconWidget)

UPanSkillIconWidget::UPanSkillIconWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanSkillIconWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	SetVisibility(ESlateVisibility::Hidden);

	if (ULyraPawnExtensionComponent* PawnExtensionComponent = ULyraPawnExtensionComponent::FindPawnExtensionComponent(GetOwningPlayerPawn()))
	{
		PawnExtensionComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	}
}

void UPanSkillIconWidget::NativeDestruct()
{
	if (ULyraAbilitySystemComponent* LyraASC = Cast<ULyraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn())))
	{
		LyraASC->AbilityChangedDelegate.Remove(AbilityDelegateHandle);
		AbilityDelegateHandle.Reset();
	}
	
	Super::NativeDestruct();
}

void UPanSkillIconWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshUI();
}

void UPanSkillIconWidget::RefreshUI()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (ASC == nullptr)
		return;

	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(CachedAbilitySpecHandle);
	if (AbilitySpec == nullptr)
		return;

	if (AbilitySpec->Ability->CanActivateAbility(AbilitySpec->Handle, ASC->AbilityActorInfo.Get()))
	{
		Image_SkillIcon->SetColorAndOpacity(FLinearColor::White);
	}
	else
	{
		Image_SkillIcon->SetColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f));
	}
	
	float CooldownTime = AbilitySpec->Ability->GetCooldownTimeRemaining(ASC->AbilityActorInfo.Get());
	if (CooldownTime > 0.f)
	{
		Text_Cooldown->SetText(UKismetTextLibrary::Conv_DoubleToText(CooldownTime, HalfFromZero, false, true, 1, 2, 1, 1));
		HorizontalBox_Cooldown->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		HorizontalBox_Cooldown->SetVisibility(ESlateVisibility::Hidden);
	}
}

FGameplayAbilitySpec* UPanSkillIconWidget::FindAbilitySpecFromHandle(FGameplayAbilitySpecHandle AbilitySpecHandle)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn()))
	{
		if (FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(AbilitySpecHandle))
		{
			return AbilitySpec;
		}
	}
	return nullptr;
}

void UPanSkillIconWidget::OnAbilitySystemInitialized()
{
	if (ULyraAbilitySystemComponent* LyraASC = Cast<ULyraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn())))
	{
		for (const FGameplayAbilitySpec& AbilitySpec : LyraASC->GetActivatableAbilities())
		{
			if (AbilitySpec.Ability->AbilityTags.HasTagExact(SkillSlotTag))
			{
				CachedAbilitySpecHandle = AbilitySpec.Handle;
				InitializeUI();
				break;
			}
		}
		
		AbilityDelegateHandle = LyraASC->AbilityChangedDelegate.AddUObject(this, &ThisClass::OnAbilityChanged);
	}
}

void UPanSkillIconWidget::OnAbilityChanged(FGameplayAbilitySpecHandle AbilitySpecHandle, bool bGiven)
{
	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (AbilitySpec && AbilitySpec->Ability->AbilityTags.HasTagExact(SkillSlotTag))
	{
		CachedAbilitySpecHandle = bGiven ? AbilitySpec->Handle : FGameplayAbilitySpecHandle();
		InitializeUI();
	}
}

void UPanSkillIconWidget::InitializeUI()
{
	if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(CachedAbilitySpecHandle))
	{
		if (ULyraGameplayAbility* Ability = Cast<ULyraGameplayAbility>(AbilitySpec->Ability))
		{
			Image_SkillIcon->SetBrushFromTexture(Ability->Icon);
		}
		
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}
