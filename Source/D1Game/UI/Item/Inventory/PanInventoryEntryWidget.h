#pragma once

#include "UI/Item/PanItemEntryWidget.h"
#include "PanInventoryEntryWidget.generated.h"

class USizeBox;
class UPanItemInstance;
class UPanInventorySlotsWidget;

UCLASS()
class UPanInventoryEntryWidget : public UPanItemEntryWidget
{
	GENERATED_BODY()
	
public:
	UPanInventoryEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(UPanInventorySlotsWidget* InSlotsWidget, UPanItemInstance* InItemInstance, int32 InItemCount);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	UPROPERTY()
	TObjectPtr<UPanInventorySlotsWidget> SlotsWidget;

	FIntPoint CachedFromSlotPos = FIntPoint::ZeroValue;
	FVector2D CachedDeltaWidgetPos = FVector2D::ZeroVector;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
};
