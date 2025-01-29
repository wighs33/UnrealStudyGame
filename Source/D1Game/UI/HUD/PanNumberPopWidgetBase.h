#pragma once

#include "Blueprint/UserWidget.h"
#include "PanNumberPopWidgetBase.generated.h"

UCLASS()
class UPanNumberPopWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanNumberPopWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void InitializeUI(int32 InNumberToDisplay, FLinearColor InColor);
};
