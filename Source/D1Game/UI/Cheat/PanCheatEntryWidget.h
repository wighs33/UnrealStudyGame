#pragma once

#include "Blueprint/UserWidget.h"
#include "PanCheatEntryWidget.generated.h"

class USizeBox;
class UImage;
class UButton;
class UCommonTextBlock;
class UPanItemTemplate;

UENUM(BlueprintType)
enum class EPanCheatEntryType : uint8
{
	None,
	PrimaryWeapon,
	SecondaryWeapon,
	Armor,
	Utility,
	Animation
};

UCLASS()
class UPanCheatEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanCheatEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	
public:
	void InitializeUI(EPanCheatEntryType InCheatEntryType, TSubclassOf<UPanItemTemplate> InItemTemplateClass, TSoftObjectPtr<UAnimMontage> InAnimMontage);

private:
	UFUNCTION()
	void OnButtonClicked();
	
public:
	UPROPERTY()
	EPanCheatEntryType CheatEntryType = EPanCheatEntryType::None;

	UPROPERTY()
	TSubclassOf<UPanItemTemplate> ItemTemplateClass;

	UPROPERTY()
	TSoftObjectPtr<UAnimMontage> AnimMontage;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Entry;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Entry;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Entry;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Entry;
};
