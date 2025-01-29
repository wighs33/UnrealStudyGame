#include "PanEquipmentSlotWidget.h"

#include "PanEquipmentEntryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanEquipmentSlotWidget)

UPanEquipmentSlotWidget::UPanEquipmentSlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanEquipmentSlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	OnDragEnded();
}

void UPanEquipmentSlotWidget::OnDragEnded()
{
	bAlreadyHovered = false;
}
