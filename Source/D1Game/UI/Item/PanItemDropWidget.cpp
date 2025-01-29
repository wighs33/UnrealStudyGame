#include "PanItemDropWidget.h"

#include "PanItemDragDrop.h"
#include "PanItemEntryWidget.h"
#include "Item/Managers/PanItemManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemDropWidget)

UPanItemDropWidget::UPanItemDropWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

bool UPanItemDropWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (UPanItemDragDrop* ItemDragDrop = Cast<UPanItemDragDrop>(InOperation))
	{
		if (UPanItemEntryWidget* FromEntryWidget = ItemDragDrop->FromEntryWidget)
		{
			FromEntryWidget->RefreshWidgetOpacity(true);
		}

		if (UPanItemManagerComponent* ItemManager = GetOwningPlayer()->FindComponentByClass<UPanItemManagerComponent>())
		{
			if (ItemDragDrop->FromInventoryManager)
			{
				ItemManager->Server_DropItemFromInventory(ItemDragDrop->FromInventoryManager, ItemDragDrop->FromItemSlotPos);
			}
			else if (ItemDragDrop->FromEquipmentManager)
			{
				ItemManager->Server_DropItemFromEquipment(ItemDragDrop->FromEquipmentManager, ItemDragDrop->FromEquipmentSlotType);
			}
		}
	}

	return true;
}
