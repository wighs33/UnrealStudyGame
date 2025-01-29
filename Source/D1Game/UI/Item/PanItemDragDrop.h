#pragma once

#include "PanDefine.h"
#include "Blueprint/DragDropOperation.h"
#include "PanItemDragDrop.generated.h"

class UPanItemInstance;
class UPanItemEntryWidget;
class UPanEquipmentManagerComponent;
class UPanInventoryManagerComponent;

UCLASS()
class UPanItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UPanItemDragDrop(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UPROPERTY()
	TObjectPtr<UPanInventoryManagerComponent> FromInventoryManager;

	FIntPoint FromItemSlotPos = FIntPoint::ZeroValue;

public:
	UPROPERTY()
	TObjectPtr<UPanEquipmentManagerComponent> FromEquipmentManager;

	EEquipmentSlotType FromEquipmentSlotType = EEquipmentSlotType::Count;

public:
	UPROPERTY()
	TObjectPtr<UPanInventoryManagerComponent> ToInventoryManager;

	FIntPoint ToItemSlotPos = FIntPoint::ZeroValue;

public:
	UPROPERTY()
	TObjectPtr<UPanEquipmentManagerComponent> ToEquipmentManager;

	EEquipmentSlotType ToEquipmentSlotType = EEquipmentSlotType::Count;
	
public:
	UPROPERTY()
	TObjectPtr<UPanItemEntryWidget> FromEntryWidget;
	
	UPROPERTY()
	TObjectPtr<UPanItemInstance> FromItemInstance;
	
	FVector2D DeltaWidgetPos = FVector2D::ZeroVector;
};
