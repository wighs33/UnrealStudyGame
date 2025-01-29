#include "PanSkillStatHoverWidget.h"

#include "PanSkillStatHoverEntryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanSkillStatHoverWidget)

UPanSkillStatHoverWidget::UPanSkillStatHoverWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanSkillStatHoverWidget::RefreshUI(FText Name, FText Description)
{
	Widget_HoverEntry->RefreshUI(Name, Description);
}
