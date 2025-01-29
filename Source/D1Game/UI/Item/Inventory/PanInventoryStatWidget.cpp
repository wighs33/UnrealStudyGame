#include "PanInventoryStatWidget.h"

#include "Data/PanUIData.h"
#include "UI/Item/PanSkillStatHoverWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanInventoryStatWidget)

UPanInventoryStatWidget::UPanInventoryStatWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanInventoryStatWidget::NativeDestruct()
{
	if (HoverWidget)
	{
		HoverWidget->RemoveFromParent();
		HoverWidget = nullptr;
	}
	
	Super::NativeDestruct();
}

void UPanInventoryStatWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	const FPanUIInfo& UIInfo = UPanUIData::Get().GetTagUIInfo(StatTag);
	if (UIInfo.Title.IsEmpty() == false && UIInfo.Content.IsEmpty() == false)
	{
		if (HoverWidget == nullptr)
		{
			TSubclassOf<UPanSkillStatHoverWidget> HoverWidgetClass = UPanUIData::Get().SkillStatHoverWidget;
			HoverWidget = CreateWidget<UPanSkillStatHoverWidget>(GetOwningPlayer(), HoverWidgetClass);
		}

		if (HoverWidget)
		{
		
			HoverWidget->RefreshUI(UIInfo.Title, UIInfo.Content);
			HoverWidget->AddToViewport();
		}
	}
}

FReply UPanInventoryStatWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (HoverWidget)
	{
		HoverWidget->SetPosition(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	
	return Reply;
}

void UPanInventoryStatWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (HoverWidget)
	{
		HoverWidget->RemoveFromParent();
		HoverWidget = nullptr;
	}
}
