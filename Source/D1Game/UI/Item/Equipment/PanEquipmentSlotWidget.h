#pragma once

#include "Blueprint/UserWidget.h"
#include "PanEquipmentSlotWidget.generated.h"

class UPanEquipmentEntryWidget;
class UPanEquipmentManagerComponent;

UCLASS()
class UPanEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanEquipmentSlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void OnDragEnded();

protected:
	UPROPERTY()
	TObjectPtr<UPanEquipmentManagerComponent> EquipmentManager;

protected:
	bool bAlreadyHovered = false;
};
