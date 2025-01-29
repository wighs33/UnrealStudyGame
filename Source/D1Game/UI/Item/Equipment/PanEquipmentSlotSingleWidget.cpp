#include "PanEquipmentSlotSingleWidget.h"

#include "PanEquipmentSlotsWidget.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Item/PanItemInstance.h"
#include "Item/Fragments/PanItemFragment_Equipable_Armor.h"
#include "Item/Fragments/PanItemFragment_Equipable_Utility.h"
#include "Item/Managers/PanEquipManagerComponent.h"
#include "Item/Managers/PanEquipmentManagerComponent.h"
#include "Item/Managers/PanInventoryManagerComponent.h"
#include "Item/Managers/PanItemManagerComponent.h"
#include "System/LyraAssetManager.h"
#include "UI/Item/PanItemDragDrop.h"
#include "UI/Item/Equipment/PanEquipmentEntryWidget.h"
#include "UI/Item/Inventory/PanInventoryEntryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanEquipmentSlotSingleWidget)

UPanEquipmentSlotSingleWidget::UPanEquipmentSlotSingleWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanEquipmentSlotSingleWidget::Init(EArmorType InArmorType, UPanEquipmentManagerComponent* InEquipmentManager)
{
	check(InArmorType != EArmorType::Count && InEquipmentManager != nullptr);

	EquipmentSingleSlotType = EEquipmentSingleSlotType::Armor;
	ArmorType = InArmorType;
	EquipmentManager = InEquipmentManager;
}

void UPanEquipmentSlotSingleWidget::Init(EUtilitySlotType InUtilitySlotType, UPanEquipmentManagerComponent* InEquipmentManager)
{
	check(InUtilitySlotType != EUtilitySlotType::Count && InEquipmentManager != nullptr);

	EquipmentSingleSlotType = EEquipmentSingleSlotType::Utility;
	UtilitySlotType = InUtilitySlotType;
	EquipmentManager = InEquipmentManager;
}

void UPanEquipmentSlotSingleWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (Image_BaseIcon)
	{
		Image_BaseIcon->SetBrushFromTexture(BaseIconTexture, true);
	}
}

bool UPanEquipmentSlotSingleWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
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
	EEquipmentSlotType ToEquipmentSlotType = GetEquipmentSlotType();
	if (ToEquipmentSlotType == EEquipmentSlotType::Count)
		return false;
	
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
	
	if (bIsValid)
	{
		Image_Red->SetVisibility(ESlateVisibility::Hidden);
		Image_Green->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Image_Red->SetVisibility(ESlateVisibility::Visible);
		Image_Green->SetVisibility(ESlateVisibility::Hidden);
	}
	return true;
}

bool UPanEquipmentSlotSingleWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
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

	EEquipmentSlotType ToEquipmentSlotType = GetEquipmentSlotType();
	if (ToEquipmentSlotType == EEquipmentSlotType::Count)
		return false;

	const UPanItemFragment* FromItemFragment = nullptr;
	switch (EquipmentSingleSlotType)
	{
	case EEquipmentSingleSlotType::Armor:	FromItemFragment = FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Armor>();	break;
	case EEquipmentSingleSlotType::Utility:	FromItemFragment = FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Utility>();	break;
	}
	if (FromItemFragment == nullptr)
		return false;
	
	if (UPanInventoryManagerComponent* FromInventoryManager = ItemDragDrop->FromInventoryManager)
	{
		ItemManager->Server_InventoryToEquipment(FromInventoryManager, ItemDragDrop->FromItemSlotPos, EquipmentManager, ToEquipmentSlotType);
	}
	else if (UPanEquipmentManagerComponent* FromEquipmentManager = ItemDragDrop->FromEquipmentManager)
	{
		ItemManager->Server_EquipmentToEquipment(FromEquipmentManager, ItemDragDrop->FromEquipmentSlotType, EquipmentManager, ToEquipmentSlotType);
	}
	
	return true;
}

void UPanEquipmentSlotSingleWidget::OnDragEnded()
{
	Super::OnDragEnded();

	Image_Red->SetVisibility(ESlateVisibility::Hidden);
	Image_Green->SetVisibility(ESlateVisibility::Hidden);
}

void UPanEquipmentSlotSingleWidget::OnEquipmentEntryChanged(UPanItemInstance* InItemInstance, int32 InItemCount)
{
	if (EntryWidget)
	{
		Overlay_Entry->RemoveChild(EntryWidget);
		EntryWidget = nullptr;
	}
	
	if (InItemInstance)
	{
		TSubclassOf<UPanEquipmentEntryWidget> EntryWidgetClass = UPanUIData::Get().EquipmentEntryWidgetClass;
		EntryWidget = CreateWidget<UPanEquipmentEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		
		UOverlaySlot* OverlaySlot = Overlay_Entry->AddChildToOverlay(EntryWidget);
		OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		
		EntryWidget->Init(InItemInstance, InItemCount, GetEquipmentSlotType(), EquipmentManager);

		Image_BaseIcon->SetRenderOpacity(0.f);
	}
	else
	{
		Image_BaseIcon->SetRenderOpacity(1.f);
	}
}

EEquipmentSlotType UPanEquipmentSlotSingleWidget::GetEquipmentSlotType() const
{
	switch (EquipmentSingleSlotType)
	{
	case EEquipmentSingleSlotType::Armor:	return UPanEquipManagerComponent::ConvertToEquipmentSlotType(ArmorType);
	case EEquipmentSingleSlotType::Utility:	return UPanEquipManagerComponent::ConvertToEquipmentSlotType(UtilitySlotType);
	}

	return EEquipmentSlotType::Count;
}
