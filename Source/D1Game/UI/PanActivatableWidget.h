#pragma once

#include "CommonActivatableWidget.h"
#include "PanActivatableWidget.generated.h"

UCLASS()
class UPanActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	UPanActivatableWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
	// TODO: Switch to InputAction
	UPROPERTY(EditDefaultsOnly)
	FKey DeactivateKey;
};
