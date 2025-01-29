#include "PanEquipmentEntryWidget.h"

#include "PanEquipmentSlotsWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/PanItemData.h"
#include "Data/PanUIData.h"
#include "Item/PanItemInstance.h"
#include "Item/PanItemTemplate.h"
#include "UI/Item/PanItemDragDrop.h"
#include "UI/Item/PanItemDragWidget.h"
#include "Item/Managers/PanItemManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanEquipmentEntryWidget)

UPanEquipmentEntryWidget::UPanEquipmentEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanEquipmentEntryWidget::Init(UPanItemInstance* InItemInstance, int32 InItemCount, EEquipmentSlotType InEquipmentSlotType, UPanEquipmentManagerComponent* InEquipmentManager)
{
	if (InEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || InEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || InEquipmentSlotType == EEquipmentSlotType::Count)
		return;
	
	RefreshUI(InItemInstance, InItemCount);
	
	EquipmentSlotType = InEquipmentSlotType;
	EquipmentManager = InEquipmentManager;
}

void UPanEquipmentEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	FIntPoint UnitInventorySlotSize = UPanUIData::Get().UnitInventorySlotSize;
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());

	TSubclassOf<UPanItemDragWidget> DragWidgetClass = UPanUIData::Get().DragWidgetClass;
	UPanItemDragWidget* ItemDragWidget = CreateWidget<UPanItemDragWidget>(GetOwningPlayer(), DragWidgetClass);
	FVector2D DragWidgetSize = FVector2D(ItemTemplate.SlotCount * UnitInventorySlotSize);
	ItemDragWidget->Init(DragWidgetSize, ItemTemplate.IconTexture, ItemCount);
	
	UPanItemDragDrop* ItemDragDrop = NewObject<UPanItemDragDrop>();
	ItemDragDrop->DefaultDragVisual = ItemDragWidget;
	ItemDragDrop->Pivot = EDragPivot::CenterCenter;
	ItemDragDrop->FromEntryWidget = this;
	ItemDragDrop->FromEquipmentManager = EquipmentManager;
	ItemDragDrop->FromEquipmentSlotType = EquipmentSlotType;
	ItemDragDrop->FromItemInstance = ItemInstance;
	ItemDragDrop->DeltaWidgetPos = (DragWidgetSize / 2.f) - (UnitInventorySlotSize / 2.f);
	OutOperation = ItemDragDrop;
}

FReply UPanEquipmentEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	if (Reply.IsEventHandled() == false && UWidgetBlueprintLibrary::IsDragDropping() == false && InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (UPanItemManagerComponent* ItemManager = GetOwningPlayer()->FindComponentByClass<UPanItemManagerComponent>())
		{
			ItemManager->Server_QuickFromEquipment(EquipmentManager, EquipmentSlotType);
			return FReply::Handled();
		}
	}

	return Reply;
}
