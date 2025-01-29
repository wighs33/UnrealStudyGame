﻿#pragma once

#include "Blueprint/UserWidget.h"
#include "PanSkillStatHoverEntryWidget.generated.h"

class URichTextBlock;
class UTextBlock;

UCLASS()
class UPanSkillStatHoverEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanSkillStatHoverEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void RefreshUI(FText Name, FText Description);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Name;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> Text_Description;
	
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Animation_FadeIn;
};
