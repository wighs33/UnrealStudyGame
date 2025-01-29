#include "PanActivatableWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanActivatableWidget)

UPanActivatableWidget::UPanActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

FReply UPanActivatableWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Reply = Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	
	if (DeactivateKey.IsValid() && InKeyEvent.GetKey() == DeactivateKey && InKeyEvent.IsRepeat() == false)
	{
		DeactivateWidget();
		return FReply::Handled();
	}
	
	return Reply;
}
