#pragma once

#include "PanDefine.h"
#include "Blueprint/UserWidget.h"
#include "PanInventoryValidWidget.generated.h"

class UImage;
class USizeBox;

UCLASS()
class UPanInventoryValidWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanInventoryValidWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;

public:
	void ChangeSlotState(ESlotState SlotState);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Red;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Green;
};
