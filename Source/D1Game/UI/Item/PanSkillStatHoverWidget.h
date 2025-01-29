#pragma once

#include "UI/PanHoverWidget.h"
#include "PanSkillStatHoverWidget.generated.h"

class UPanSkillStatHoverEntryWidget;

UCLASS()
class UPanSkillStatHoverWidget : public UPanHoverWidget
{
	GENERATED_BODY()
	
public:
	UPanSkillStatHoverWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void RefreshUI(FText Name, FText Description);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanSkillStatHoverEntryWidget> Widget_HoverEntry;
};
