#include "PanEquipmentSlotWeaponWidget.h"

#include "CommonVisibilitySwitcher.h"
#include "PanDefine.h"
#include "Item/PanItemInstance.h"
#include "Item/Managers/PanEquipmentManagerComponent.h"
#include "PanEquipmentEntryWidget.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Data/PanUIData.h"
#include "Item/Fragments/PanItemFragment_Equipable_Weapon.h"
#include "Item/Managers/PanEquipManagerComponent.h"
#include "Item/Managers/PanItemManagerComponent.h"
#include "UI/Item/PanItemDragDrop.h"
#include "UI/Item/Inventory/PanInventoryEntryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanEquipmentSlotWeaponWidget)

UPanEquipmentSlotWeaponWidget::UPanEquipmentSlotWeaponWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UPanEquipmentSlotWeaponWidget::Init(EWeaponSlotType InWeaponSlotType, UPanEquipmentManagerComponent* InEquipmentManager)
{
	check(InWeaponSlotType != EWeaponSlotType::Count && InEquipmentManager != nullptr);
	
	WeaponSlotType = InWeaponSlotType;
	EquipmentManager = InEquipmentManager;
}

void UPanEquipmentSlotWeaponWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EntryWidgets.SetNum((int32)EWeaponHandType::Count);
	SlotRedImages = { Image_Red_LeftHand, Image_Red_RightHand, Image_Red_TwoHand };
	SlotGreenImages = { Image_Green_LeftHand, Image_Green_RightHand, Image_Green_TwoHand };
	SlotOverlays = { Overlay_LeftHandEntry, Overlay_RightHandEntry, Overlay_TwoHandEntry };
}

bool UPanEquipmentSlotWeaponWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	if (bAlreadyHovered)
		return true;
	
	bAlreadyHovered = true;
	
	UPanItemDragDrop* ItemDragDrop = Cast<UPanItemDragDrop>(InOperation);
	if (ItemDragDrop == nullptr)
		return false;
	
	UPanItemInstance* FromItemInstance = ItemDragDrop->FromItemInstance;
	if (FromItemInstance == nullptr)
		return false;

	bool bIsValid = false;
	UImage* TargetImage = Image_Red_TwoHand;
	
	if (const UPanItemFragment_Equipable_Weapon* FromWeaponFragment = FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Weapon>())
	{
		EEquipmentSlotType ToEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType(FromWeaponFragment->WeaponHandType, WeaponSlotType);
	
		if (UPanInventoryManagerComponent* FromInventoryManager = ItemDragDrop->FromInventoryManager)
		{
			if (EquipmentManager->GetItemInstance(ToEquipmentSlotType))
			{
				FIntPoint OutToItemSlotPos;
				bIsValid = EquipmentManager->CanSwapEquipment(FromInventoryManager, ItemDragDrop->FromItemSlotPos, ToEquipmentSlotType, OutToItemSlotPos);
			}
			else
			{
				bIsValid = EquipmentManager->CanMoveOrMergeEquipment(FromInventoryManager, ItemDragDrop->FromItemSlotPos, ToEquipmentSlotType) > 0;
			}
		}
		else if (UPanEquipmentManagerComponent* FromEquipmentManager = ItemDragDrop->FromEquipmentManager)
		{
			if (EquipmentManager->GetItemInstance(ToEquipmentSlotType))
			{
				bIsValid = EquipmentManager->CanSwapEquipment(FromEquipmentManager, ItemDragDrop->FromEquipmentSlotType, ToEquipmentSlotType);
			}
			else
			{
				bIsValid = EquipmentManager->CanMoveOrMergeEquipment(FromEquipmentManager, ItemDragDrop->FromEquipmentSlotType, ToEquipmentSlotType) > 0;
			}
		}

		const int32 WeaponHandIndex = (int32)FromWeaponFragment->WeaponHandType;
		TargetImage = bIsValid ? SlotGreenImages[WeaponHandIndex] : SlotRedImages[WeaponHandIndex];
	}

	if (TargetImage)
	{
		TargetImage->SetVisibility(ESlateVisibility::Visible);
	}
	
	return true;
}

bool UPanEquipmentSlotWeaponWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	OnDragEnded();
	
	UPanItemDragDrop* ItemDragDrop = Cast<UPanItemDragDrop>(InOperation);
	if (ItemDragDrop == nullptr)
		return false;
	
	if (UPanItemEntryWidget* FromEntryWidget = ItemDragDrop->FromEntryWidget)
	{
		FromEntryWidget->RefreshWidgetOpacity(true);
	}
	
	UPanItemInstance* FromItemInstance = ItemDragDrop->FromItemInstance;
	if (FromItemInstance == nullptr)
		return false;

	UPanItemManagerComponent* ItemManager = GetOwningPlayer()->FindComponentByClass<UPanItemManagerComponent>();
	if (ItemManager == nullptr)
		return false;
	
	if (const UPanItemFragment_Equipable_Weapon* FromWeaponFragment = FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Weapon>())
	{
		EEquipmentSlotType ToEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType(FromWeaponFragment->WeaponHandType, WeaponSlotType);
	
		if (UPanInventoryManagerComponent* FromInventoryManager = ItemDragDrop->FromInventoryManager)
		{
			ItemManager->Server_InventoryToEquipment(FromInventoryManager, ItemDragDrop->FromItemSlotPos, EquipmentManager, ToEquipmentSlotType);
		}
		else if (UPanEquipmentManagerComponent* FromEquipmentManager = ItemDragDrop->FromEquipmentManager)
		{
			ItemManager->Server_EquipmentToEquipment(FromEquipmentManager, ItemDragDrop->FromEquipmentSlotType, EquipmentManager, ToEquipmentSlotType);
		}
	}
	return true;
}

void UPanEquipmentSlotWeaponWidget::OnDragEnded()
{
	Super::OnDragEnded();

	for (UImage* SlotGreenImage : SlotGreenImages)
	{
		SlotGreenImage->SetVisibility(ESlateVisibility::Hidden);
	}

	for (UImage* SlotRedImage : SlotRedImages)
	{
		SlotRedImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPanEquipmentSlotWeaponWidget::OnEquipmentEntryChanged(EWeaponHandType InWeaponHandType, UPanItemInstance* InItemInstance, int32 InItemCount)
{
	if (InWeaponHandType == EWeaponHandType::Count)
		return;

	const int32 WeaponHandIndex = (int32)InWeaponHandType;
	UOverlay* TargetOverlay = SlotOverlays[WeaponHandIndex];

	if (UPanEquipmentEntryWidget* EntryWidget = EntryWidgets[WeaponHandIndex])
	{
		if (UPanItemInstance* ItemInstance = EntryWidget->GetItemInstance())
		{
			if (ItemInstance == InItemInstance)
			{
				EntryWidget->RefreshItemCount(InItemCount);
				return;
			}
		}
		
		TargetOverlay->RemoveChild(EntryWidget);
		EntryWidgets[WeaponHandIndex] = nullptr;
	}

	int32 ActiveWidgetIndex = Switcher_WeaponHandEntry->GetActiveWidgetIndex();
	
	if (InItemInstance)
	{
		TSubclassOf<UPanEquipmentEntryWidget> EntryWidgetClass = UPanUIData::Get().EquipmentEntryWidgetClass;
		UPanEquipmentEntryWidget* EntryWidget = CreateWidget<UPanEquipmentEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		EntryWidgets[WeaponHandIndex] = EntryWidget;
		
		UOverlaySlot* OverlaySlot = TargetOverlay->AddChildToOverlay(EntryWidget);
		OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		
		EntryWidget->Init(InItemInstance, InItemCount, UPanEquipManagerComponent::ConvertToEquipmentSlotType(InWeaponHandType, WeaponSlotType), EquipmentManager);
		
		if (InWeaponHandType == EWeaponHandType::TwoHand)
			ActiveWidgetIndex = 1;
	}
	else
	{
		if (InWeaponHandType == EWeaponHandType::TwoHand)
			ActiveWidgetIndex = 0;
	}
	
	Switcher_WeaponHandEntry->SetActiveWidgetIndex(ActiveWidgetIndex);
}
