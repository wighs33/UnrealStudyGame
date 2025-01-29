#pragma once

#include "UI/PanActivatableWidget.h"
#include "PanCheatMenuWidget.generated.h"

class UButton;
class UPanCheatEntryWidget;
class UPanCheatListWidget;

UCLASS()
class UPanCheatMenuWidget : public UPanActivatableWidget
{
	GENERATED_BODY()
	
public:
	UPanCheatMenuWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void OnExitButtonClicked();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanCheatEntryWidget> CheatEntryWidgetClass;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanCheatListWidget> CheatList_PrimaryWeapon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanCheatListWidget> CheatList_SecondaryWeapon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanCheatListWidget> CheatList_Utility;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanCheatListWidget> CheatList_Armor;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanCheatListWidget> CheatList_Animation;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Exit;
};
