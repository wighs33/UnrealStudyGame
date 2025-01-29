#pragma once

#include "PanNumberPopWidgetBase.h"
#include "PanNumberPopWidgetDefault.generated.h"

class UTextBlock;

UCLASS()
class UPanNumberPopWidgetDefault : public UPanNumberPopWidgetBase
{
	GENERATED_BODY()
	
public:
	UPanNumberPopWidgetDefault(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeUI(int32 InNumberToDisplay, FLinearColor InColor) override;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Number;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Animation_NumberPop;
};
