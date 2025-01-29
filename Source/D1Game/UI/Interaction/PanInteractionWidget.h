#pragma once

#include "Blueprint/UserWidget.h"
#include "PanInteractionWidget.generated.h"

UCLASS()
class UPanInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanInteractionWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
