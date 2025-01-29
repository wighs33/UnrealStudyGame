#pragma once

#include "PanDefine.h"
#include "PanEquipmentSlotWidget.h"
#include "PanEquipmentSlotWeaponWidget.generated.h"

class UImage;
class UOverlay;
class UCommonVisibilitySwitcher;
class UPanItemInstance;
class UPanEquipmentEntryWidget;
class UPanEquipmentManagerComponent;

UCLASS()
class UPanEquipmentSlotWeaponWidget : public UPanEquipmentSlotWidget
{
	GENERATED_BODY()
	
public:
	UPanEquipmentSlotWeaponWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(EWeaponSlotType InWeaponSlotType, UPanEquipmentManagerComponent* InEquipmentManager);
	
protected:
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void OnDragEnded() override;

public:
	void OnEquipmentEntryChanged(EWeaponHandType InWeaponHandType, UPanItemInstance* InItemInstance, int32 InItemCount);
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UPanEquipmentEntryWidget>> EntryWidgets;
	
	UPROPERTY()
	TArray<TObjectPtr<UImage>> SlotRedImages;

	UPROPERTY()
	TArray<TObjectPtr<UImage>> SlotGreenImages;

	UPROPERTY()
	TArray<TObjectPtr<UOverlay>> SlotOverlays;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonVisibilitySwitcher> Switcher_WeaponHandEntry;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> Overlay_LeftHandEntry;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> Overlay_RightHandEntry;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> Overlay_TwoHandEntry;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Red_LeftHand;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Green_LeftHand;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Red_RightHand;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Green_RightHand;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Red_TwoHand;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Green_TwoHand;

private:
	EWeaponSlotType WeaponSlotType = EWeaponSlotType::Count;
};
