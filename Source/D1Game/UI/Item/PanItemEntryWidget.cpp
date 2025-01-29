#include "PanItemEntryWidget.h"

#include "Components/Image.h"
#include "Data/PanItemData.h"
#include "Item/PanItemInstance.h"
#include "Item/PanItemTemplate.h"
#include "PanItemDragWidget.h"
#include "PanItemHoverWidget.h"
#include "Components/TextBlock.h"
#include "Data/PanUIData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemEntryWidget)

UPanItemEntryWidget::UPanItemEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UPanItemEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Text_Count->SetText(FText::GetEmpty());
}

void UPanItemEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Image_Hover->SetVisibility(ESlateVisibility::Hidden);
}

void UPanItemEntryWidget::NativeDestruct()
{
	if (HoverWidget)
	{
		HoverWidget->RemoveFromParent();
		HoverWidget = nullptr;
	}
	
	Super::NativeDestruct();
}

void UPanItemEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	Image_Hover->SetVisibility(ESlateVisibility::Visible);

	if (HoverWidget == nullptr)
	{
		TSubclassOf<UPanItemHoverWidget> HoverWidgetClass = UPanUIData::Get().ItemHoverWidgetClass;
		HoverWidget = CreateWidget<UPanItemHoverWidget>(GetOwningPlayer(), HoverWidgetClass);
	}

	if (HoverWidget)
	{
		HoverWidget->RefreshUI(ItemInstance);
		HoverWidget->AddToViewport();
	}
}

FReply UPanItemEntryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (HoverWidget)
	{
		HoverWidget->SetPosition(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	
	return Reply;
}

void UPanItemEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	Image_Hover->SetVisibility(ESlateVisibility::Hidden);

	if (HoverWidget)
	{
		HoverWidget->RemoveFromParent();
		HoverWidget = nullptr;
	}
}

FReply UPanItemEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return Reply;
}

void UPanItemEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	RefreshWidgetOpacity(false);
}

void UPanItemEntryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	RefreshWidgetOpacity(true);
}

void UPanItemEntryWidget::RefreshWidgetOpacity(bool bClearlyVisible)
{
	SetRenderOpacity(bClearlyVisible ? 1.f : 0.5f);
}

void UPanItemEntryWidget::RefreshUI(UPanItemInstance* NewItemInstance, int32 NewItemCount)
{
	if (NewItemInstance == nullptr || NewItemCount < 1)
		return;
	
	ItemInstance = NewItemInstance;
	ItemCount = NewItemCount;

	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
	Image_Icon->SetBrushFromTexture(ItemTemplate.IconTexture, true);
	Text_Count->SetText(ItemCount <= 1 ? FText::GetEmpty() : FText::AsNumber(ItemCount));

	UTexture2D* RarityTexture = UPanUIData::Get().GetEntryRarityTexture(ItemInstance->GetItemRarity());
	Image_RarityCover->SetBrushFromTexture(RarityTexture, true);
}

void UPanItemEntryWidget::RefreshItemCount(int32 NewItemCount)
{
	if (NewItemCount < 1)
		return;
	
	ItemCount = NewItemCount;
	Text_Count->SetText(ItemCount <= 1 ? FText::GetEmpty() : FText::AsNumber(ItemCount));
}
