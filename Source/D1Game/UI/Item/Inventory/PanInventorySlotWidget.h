#pragma once

#include "Blueprint/UserWidget.h"
#include "PanInventorySlotWidget.generated.h"

class UImage;
class USizeBox;

UCLASS()
class UPanInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanInventorySlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
};
