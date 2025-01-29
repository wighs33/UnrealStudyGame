#include "PanClassEntryWidget.h"

#include "PanClassSelectionWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "PanClassSkillEntryWidget.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "Components/Button.h"
#include "Data/PanClassData.h"
#include "Player/LyraPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanClassEntryWidget)

UPanClassEntryWidget::UPanClassEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanClassEntryWidget::InitializeUI(UPanClassSelectionWidget* OwnerWidget, ECharacterClassType ClassType)
{
	CachedClassType = ClassType;
	CachedOwnerWidget = OwnerWidget;

	const FPanClassInfoEntry& ClassEntry = UPanClassData::Get().GetClassInfoEntry(ClassType);
	
	Text_ClassName->SetText(ClassEntry.ClassName);
	VerticalBox_SkillElements->ClearChildren();
	
	if (ULyraAbilitySet* AbilitySet = ClassEntry.ClassAbilitySet)
	{
		const TArray<FLyraAbilitySet_GameplayAbility>& AbilitySetAbilities = AbilitySet->GetGrantedGameplayAbilities();
		for (int i = 0; i < 2; i++)
		{
			if (AbilitySetAbilities.IsValidIndex(i))
			{
				const FLyraAbilitySet_GameplayAbility& AbilitySetAbility = AbilitySetAbilities[i];
				UPanClassSkillEntryWidget* SkillEntryWidget = CreateWidget<UPanClassSkillEntryWidget>(this, SkillEntryWidgetClass);
				SkillEntryWidget->InitializeUI(AbilitySetAbility.Ability);
				VerticalBox_SkillElements->AddChild(SkillEntryWidget);
			}
		}
	}

	Button_Class->OnClicked.AddUniqueDynamic(this, &ThisClass::OnButtonClicked);
}

void UPanClassEntryWidget::OnButtonClicked()
{
	if (ALyraPlayerState* LyraPlayerState = Cast<ALyraPlayerState>(GetOwningPlayerState()))
	{
		LyraPlayerState->Server_SelectClass(CachedClassType);
	}

	if (UPanClassSelectionWidget* ClassSelectionWidget = CachedOwnerWidget.Get())
	{
		ClassSelectionWidget->DeactivateWidget();
		ClassSelectionWidget = nullptr;
	}
}
