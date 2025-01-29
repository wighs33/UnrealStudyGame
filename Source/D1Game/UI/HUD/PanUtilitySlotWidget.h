#pragma once

#include "PanDefine.h"
#include "Blueprint/UserWidget.h"
#include "PanUtilitySlotWidget.generated.h"

class UPanItemInstance;
class UTextBlock;
class UImage;
class UPanEquipmentManagerComponent;
class UPanEquipManagerComponent;

UCLASS()
class UPanUtilitySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanUtilitySlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance, int32 ItemCount);
	void OnEquipStateChanged(EEquipState PrevEquipState, EEquipState NewEquipState);
	
public:
	UPROPERTY(EditAnywhere)
	EUtilitySlotType WidgetUtilitySlotType = EUtilitySlotType::Count;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_SlotNumber;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Highlight;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Count;

	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Animation_Highlight_In;
	
private:
	UPROPERTY()
	TObjectPtr<UPanEquipmentManagerComponent> EquipmentManager;
	
	UPROPERTY()
	TObjectPtr<UPanEquipManagerComponent> EquipManager;
	
private:
	FDelegateHandle EntryChangedDelegateHandle;
	FDelegateHandle EquipStateChangedDelegateHandle;
};
