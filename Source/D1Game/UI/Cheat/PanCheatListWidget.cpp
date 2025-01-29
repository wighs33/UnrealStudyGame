#include "PanCheatListWidget.h"

#include "PanCheatEntryWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanCheatListWidget)

UPanCheatListWidget::UPanCheatListWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanCheatListWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	Text_Title->SetText(TitleText);
}

void UPanCheatListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ScrollBox_Entries->ClearChildren();
}

void UPanCheatListWidget::AddEntry(UPanCheatEntryWidget* EntryWidget)
{
	ScrollBox_Entries->AddChild(EntryWidget);
}
