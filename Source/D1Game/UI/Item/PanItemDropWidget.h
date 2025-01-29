#pragma once

#include "Blueprint/UserWidget.h"
#include "PanItemDropWidget.generated.h"

UCLASS()
class UPanItemDropWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanItemDropWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
