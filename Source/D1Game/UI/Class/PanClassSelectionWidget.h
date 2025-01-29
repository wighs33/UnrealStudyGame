#pragma once

#include "UI/PanActivatableWidget.h"
#include "PanClassSelectionWidget.generated.h"

class UButton;
class UVerticalBox;
class UPanClassEntryWidget;

UCLASS()
class UPanClassSelectionWidget : public UPanActivatableWidget
{
	GENERATED_BODY()
	
public:
	UPanClassSelectionWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnExitButtonClicked();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanClassEntryWidget> ClassEntryWidgetClass;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_ClassElements;
};
