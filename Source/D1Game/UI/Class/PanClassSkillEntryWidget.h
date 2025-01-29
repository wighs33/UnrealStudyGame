#pragma once

#include "Blueprint/UserWidget.h"
#include "PanClassSkillEntryWidget.generated.h"

class URichTextBlock;
class UImage;
class UTextBlock;
class ULyraGameplayAbility;

UCLASS()
class UPanClassSkillEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanClassSkillEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void InitializeUI(const TSubclassOf<ULyraGameplayAbility>& AbilityClass);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Skill;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_SkillName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> Text_SkillDescription;
};
