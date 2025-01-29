#include "PanItemHoverWidget.h"

#include "PanItemHoverEntryWidget.h"
#include "Item/PanItemInstance.h"
#include "Item/Managers/PanEquipmentManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemHoverWidget)

UPanItemHoverWidget::UPanItemHoverWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanItemHoverWidget::RefreshUI(const UPanItemInstance* HoveredItemInstance)
{
	const UPanItemInstance* PairItemInstance = nullptr;
	if (UPanEquipmentManagerComponent* EquipmentManager = GetOwningPlayerPawn()->GetComponentByClass<UPanEquipmentManagerComponent>())
	{
		EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;
		PairItemInstance = EquipmentManager->FindPairItemInstance(HoveredItemInstance, EquipmentSlotType);
	}
	
	HoverWidget_Left->RefreshUI(HoveredItemInstance);
	HoverWidget_Right->RefreshUI(PairItemInstance);
}
