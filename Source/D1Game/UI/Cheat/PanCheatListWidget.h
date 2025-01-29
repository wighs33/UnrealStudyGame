#pragma once

#include "Blueprint/UserWidget.h"
#include "PanCheatListWidget.generated.h"

class UTextBlock;
class UPanCheatEntryWidget;
class UScrollBox;

UCLASS()
class UPanCheatListWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanCheatListWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	void AddEntry(UPanCheatEntryWidget* EntryWidget);

protected:
	UPROPERTY(EditAnywhere)
	FText TitleText;
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Title;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_Entries;
};
