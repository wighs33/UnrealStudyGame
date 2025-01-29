#pragma once

#include "Item/PanItemInstance.h"
#include "UI/PanHoverWidget.h"
#include "PanItemHoverWidget.generated.h"

class UPanEquipmentManagerComponent;
class UPanItemInstance;
class UCanvasPanel;
class UHorizontalBox;
class UPanItemHoverEntryWidget;

UCLASS()
class UPanItemHoverWidget : public UPanHoverWidget
{
	GENERATED_BODY()
	
public:
	UPanItemHoverWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void RefreshUI(const UPanItemInstance* HoveredItemInstance);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanItemHoverEntryWidget> HoverWidget_Left;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanItemHoverEntryWidget> HoverWidget_Right;
};
