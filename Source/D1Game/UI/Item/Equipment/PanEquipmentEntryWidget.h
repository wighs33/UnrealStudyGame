#pragma once

#include "PanDefine.h"
#include "UI/Item/PanItemEntryWidget.h"
#include "PanEquipmentEntryWidget.generated.h"

class UPanItemInstance;
class UPanEquipmentManagerComponent;

UCLASS()
class UPanEquipmentEntryWidget : public UPanItemEntryWidget
{
	GENERATED_BODY()
	
public:
	UPanEquipmentEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(UPanItemInstance* InItemInstance, int32 InItemCount, EEquipmentSlotType InEquipmentSlotType, UPanEquipmentManagerComponent* InEquipmentManager);
	
protected:
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	UPROPERTY()
	TObjectPtr<UPanEquipmentManagerComponent> EquipmentManager;

	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;
};
