#include "PanClassSelectionWidget.h"

#include "PanClassEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Data/PanClassData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanClassSelectionWidget)

UPanClassSelectionWidget::UPanClassSelectionWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanClassSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	VerticalBox_ClassElements->ClearChildren();

	const int32 ClassCount = (int32)ECharacterClassType::Count;
	for (int i = 0; i < ClassCount; i++)
	{
		UPanClassEntryWidget* ClassEntryWidget = CreateWidget<UPanClassEntryWidget>(this, ClassEntryWidgetClass);
		ClassEntryWidget->InitializeUI(this, (ECharacterClassType)i);
		VerticalBox_ClassElements->AddChild(ClassEntryWidget);
	}
}

void UPanClassSelectionWidget::OnExitButtonClicked()
{
	DeactivateWidget();
}
