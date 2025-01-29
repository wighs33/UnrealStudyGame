#include "PanInventorySlotWidget.h"

#include "Components/SizeBox.h"
#include "Data/PanUIData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanInventorySlotWidget)

UPanInventorySlotWidget::UPanInventorySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanInventorySlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	FIntPoint UnitInventorySlotSize = UPanUIData::Get().UnitInventorySlotSize;
	SizeBox_Root->SetWidthOverride(UnitInventorySlotSize.X);
	SizeBox_Root->SetHeightOverride(UnitInventorySlotSize.Y);
}
