#pragma once

#include "PanDefine.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PanEquipmentSlotsWidget.generated.h"

class UCommonVisibilitySwitcher;
class UPanItemInstance;
class UPanItemSlotWidget;
class UPanEquipmentSlotWeaponWidget;
class UPanEquipmentSlotSingleWidget;
class UPanEquipManagerComponent;
class UPanEquipmentManagerComponent;

USTRUCT(BlueprintType)
struct FEquipmentInitializeMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UPanEquipmentManagerComponent> EquipmentManager;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UPanEquipManagerComponent> EquipManager;
};

UCLASS()
class UPanEquipmentSlotsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanEquipmentSlotsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	void ConstructUI(FGameplayTag Channel, const FEquipmentInitializeMessage& Message);
	void DestructUI();
	
	void OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance, int32 ItemCount);
	void OnEquipStateChanged(EEquipState PrevEquipState, EEquipState NewEquipState);

public:
	UPROPERTY(EditAnywhere, meta=(Categories="Message"))
	FGameplayTag MessageChannelTag;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UPanEquipmentSlotWeaponWidget>> WeaponSlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UPanEquipmentSlotSingleWidget>> ArmorSlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UPanEquipmentSlotSingleWidget>> UtilitySlotWidgets;
	
	UPROPERTY()
	TObjectPtr<UPanEquipmentManagerComponent> EquipmentManager;
	
	UPROPERTY()
	TObjectPtr<UPanEquipManagerComponent> EquipManager;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotWeaponWidget> Widget_Weapon_Primary;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotWeaponWidget> Widget_Weapon_Secondary;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Armor_Head;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Armor_Chest;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Armor_Legs;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Armor_Hand;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Armor_Foot;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Utility_Primary;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Utility_Secondary;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Utility_Tertiary;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanEquipmentSlotSingleWidget> Widget_Utility_Quaternary;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonVisibilitySwitcher> Switcher_Weapon_Primary;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonVisibilitySwitcher> Switcher_Weapon_Secondary;

private:
	FDelegateHandle EntryChangedDelegateHandle;
	FDelegateHandle EquipStateChangedDelegateHandle;
	FGameplayMessageListenerHandle MessageListenerHandle;
};
