#include "PanInventoryEntryWidget.h"

#include "PanInventorySlotsWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/SizeBox.h"
#include "Data/PanItemData.h"
#include "Data/PanUIData.h"
#include "Item/PanItemInstance.h"
#include "Item/Fragments/PanItemFragment_Equipable.h"
#include "Item/Managers/PanInventoryManagerComponent.h"
#include "Item/Managers/PanItemManagerComponent.h"
#include "UI/Item/PanItemDragDrop.h"
#include "UI/Item/PanItemDragWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanInventoryEntryWidget)

UPanInventoryEntryWidget::UPanInventoryEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanInventoryEntryWidget::Init(UPanInventorySlotsWidget* InSlotsWidget, UPanItemInstance* InItemInstance, int32 InItemCount)
{
	if (InSlotsWidget == nullptr || InItemInstance == nullptr)
		return;
	
	SlotsWidget = InSlotsWidget;

	FIntPoint UnitInventorySlotSize = UPanUIData::Get().UnitInventorySlotSize;
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(InItemInstance->GetItemTemplateID());

	FVector2D WidgetSize = FVector2D(ItemTemplate.SlotCount.X * UnitInventorySlotSize.X, ItemTemplate.SlotCount.Y * UnitInventorySlotSize.Y);
	SizeBox_Root->SetWidthOverride(WidgetSize.X);
	SizeBox_Root->SetHeightOverride(WidgetSize.Y);
	
	RefreshUI(InItemInstance, InItemCount);
}

FReply UPanInventoryEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	FIntPoint UnitInventorySlotSize = UPanUIData::Get().UnitInventorySlotSize;
	
	FVector2D MouseWidgetPos = SlotsWidget->GetSlotContainerGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	FVector2D ItemWidgetPos = SlotsWidget->GetSlotContainerGeometry().AbsoluteToLocal(InGeometry.LocalToAbsolute(UnitInventorySlotSize / 2.f));
	FIntPoint ItemSlotPos = FIntPoint(ItemWidgetPos.X / UnitInventorySlotSize.X, ItemWidgetPos.Y / UnitInventorySlotSize.Y);
	
	CachedFromSlotPos = ItemSlotPos;
	CachedDeltaWidgetPos = MouseWidgetPos - ItemWidgetPos;
	
	if (Reply.IsEventHandled() == false && UWidgetBlueprintLibrary::IsDragDropping() == false && InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		UPanItemManagerComponent* ItemManager = GetOwningPlayer()->FindComponentByClass<UPanItemManagerComponent>();
		UPanInventoryManagerComponent* FromInventoryManager = SlotsWidget->GetInventoryManager();

		if (ItemManager && FromInventoryManager)
		{
			ItemManager->Server_QuickFromInventory(FromInventoryManager, ItemSlotPos);
			return FReply::Handled();
		}
	}
	
	return Reply;
}

void UPanInventoryEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	FIntPoint UnitInventorySlotSize = UPanUIData::Get().UnitInventorySlotSize;
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());

	TSubclassOf<UPanItemDragWidget> DragWidgetClass = UPanUIData::Get().DragWidgetClass;
	UPanItemDragWidget* DragWidget = CreateWidget<UPanItemDragWidget>(GetOwningPlayer(), DragWidgetClass);
	FVector2D DragWidgetSize = FVector2D(ItemTemplate.SlotCount.X * UnitInventorySlotSize.X, ItemTemplate.SlotCount.Y * UnitInventorySlotSize.Y);
	DragWidget->Init(DragWidgetSize, ItemTemplate.IconTexture, ItemCount);
	
	UPanItemDragDrop* DragDrop = NewObject<UPanItemDragDrop>();
	DragDrop->DefaultDragVisual = DragWidget;
	DragDrop->Pivot = EDragPivot::TopLeft;
	DragDrop->Offset = -((CachedDeltaWidgetPos + UnitInventorySlotSize / 2.f) / DragWidgetSize);
	DragDrop->FromEntryWidget = this;
	DragDrop->FromInventoryManager = SlotsWidget->GetInventoryManager();
	DragDrop->FromItemSlotPos = CachedFromSlotPos;
	DragDrop->FromItemInstance = ItemInstance;
	DragDrop->DeltaWidgetPos = CachedDeltaWidgetPos;
	OutOperation = DragDrop;
}
