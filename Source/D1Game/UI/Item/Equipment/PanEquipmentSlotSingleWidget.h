#pragma once

#include "PanDefine.h"
#include "PanEquipmentSlotWidget.h"
#include "PanEquipmentSlotSingleWidget.generated.h"

class UImage;
class UOverlay;
class UPanItemInstance;
class UPanEquipmentEntryWidget;
class UPanEquipmentManagerComponent;

UENUM()
enum class EEquipmentSingleSlotType : uint8
{
	None,
	Armor,
	Utility,
};

UCLASS()
class UPanEquipmentSlotSingleWidget : public UPanEquipmentSlotWidget
{
	GENERATED_BODY()
	
public:
	UPanEquipmentSlotSingleWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(EArmorType InArmorType, UPanEquipmentManagerComponent* InEquipmentManager);
	void Init(EUtilitySlotType InUtilitySlotType, UPanEquipmentManagerComponent* InEquipmentManager);
	
protected:
	virtual void NativePreConstruct() override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void OnDragEnded() override;

public:
	void OnEquipmentEntryChanged(UPanItemInstance* InItemInstance, int32 InItemCount);

private:
	EEquipmentSlotType GetEquipmentSlotType() const;
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> BaseIconTexture;
	
private:
	UPROPERTY()
	TObjectPtr<UPanEquipmentEntryWidget> EntryWidget;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> Overlay_Entry;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_BaseIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Red;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Green;

private:
	EEquipmentSingleSlotType EquipmentSingleSlotType = EEquipmentSingleSlotType::None;
	EArmorType ArmorType = EArmorType::Count;
	EUtilitySlotType UtilitySlotType = EUtilitySlotType::Count;
};
