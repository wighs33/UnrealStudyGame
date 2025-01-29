#pragma once

#include "PanDefine.h"
#include "Blueprint/UserWidget.h"
#include "Item/PanItemInstance.h"
#include "PanWeaponSlotWidget.generated.h"

class UImage;
class UOverlay;
class UTextBlock;
class UCommonVisibilitySwitcher;
class UPanEquipManagerComponent;
class UPanEquipmentManagerComponent;

UCLASS()
class UPanWeaponSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanWeaponSlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance, int32 ItemCount);
	void OnEquipStateChanged(EEquipState PrevEquipState, EEquipState NewEquipState);
	
public:
	UPROPERTY(EditAnywhere)
	EWeaponSlotType WidgetWeaponSlotType = EWeaponSlotType::Count;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonVisibilitySwitcher> Switcher_Slots;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_SlotNumber;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_OneSlot;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_OneSlot_Count;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_TwoSlot_Left;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_TwoSlot_Right;

private:
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Animation_ExpandSlot;
	
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Animation_ShowCrossLine;

private:
	UPROPERTY()
	TObjectPtr<UPanEquipmentManagerComponent> EquipmentManager;
	
	UPROPERTY()
	TObjectPtr<UPanEquipManagerComponent> EquipManager;
	
private:
	FDelegateHandle EntryChangedDelegateHandle;
	FDelegateHandle EquipStateChangedDelegateHandle;
};
