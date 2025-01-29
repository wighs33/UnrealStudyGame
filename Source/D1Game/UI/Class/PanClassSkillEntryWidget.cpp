#include "PanClassSkillEntryWidget.h"

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanClassSkillEntryWidget)

UPanClassSkillEntryWidget::UPanClassSkillEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanClassSkillEntryWidget::InitializeUI(const TSubclassOf<ULyraGameplayAbility>& AbilityClass)
{
	if (ULyraGameplayAbility* Ability = AbilityClass.GetDefaultObject())
	{
		Image_Skill->SetBrushFromTexture(Ability->Icon);
		Text_SkillName->SetText(Ability->Name);
		Text_SkillDescription->SetText(Ability->Description);
	}
}
