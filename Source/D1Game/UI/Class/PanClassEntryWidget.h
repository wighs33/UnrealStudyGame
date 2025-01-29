#pragma once

#include "PanDefine.h"
#include "Blueprint/UserWidget.h"
#include "PanClassEntryWidget.generated.h"

class UImage;
class UButton;
class UTextBlock;
class UVerticalBox;
class UPanClassSelectionWidget;
class UPanClassSkillEntryWidget;

UCLASS()
class UPanClassEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanClassEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION()
	void InitializeUI(UPanClassSelectionWidget* OwnerWidget, ECharacterClassType ClassType);

private:
	UFUNCTION()
	void OnButtonClicked();
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanClassSkillEntryWidget> SkillEntryWidgetClass;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Class;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_ClassName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_SkillElements;

private:
	UPROPERTY()
	ECharacterClassType CachedClassType = ECharacterClassType::Count;

	UPROPERTY()
	TWeakObjectPtr<UPanClassSelectionWidget> CachedOwnerWidget;
};
