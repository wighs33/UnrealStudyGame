#include "PanUtilitySlotWidget.h"

#include "CommonVisibilitySwitcher.h"
#include "Character/LyraCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/PanItemData.h"
#include "Item/PanItemInstance.h"
#include "Item/PanItemTemplate.h"
#include "Item/Managers/PanEquipManagerComponent.h"
#include "Item/Managers/PanEquipmentManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanUtilitySlotWidget)

UPanUtilitySlotWidget::UPanUtilitySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanUtilitySlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	FText SlotNumber = FText::AsNumber((int32)EWeaponSlotType::Count + (int32)WidgetUtilitySlotType + 1);
	Text_SlotNumber->SetText(SlotNumber);
}

void UPanUtilitySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetOwningPlayerPawn());
	if (LyraCharacter == nullptr)
		return;
	
	EquipManager = LyraCharacter->GetComponentByClass<UPanEquipManagerComponent>();
	EquipmentManager = LyraCharacter->GetComponentByClass<UPanEquipmentManagerComponent>();
	if (EquipManager == nullptr || EquipmentManager == nullptr)
		return;

	const TArray<FPanEquipmentEntry>& Entries = EquipmentManager->GetAllEntries();
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		const FPanEquipmentEntry& Entry = Entries[i];
		if (UPanItemInstance* ItemInstance = Entry.GetItemInstance())
		{
			OnEquipmentEntryChanged((EEquipmentSlotType)i, ItemInstance, Entry.GetItemCount());
		}
	}
	EntryChangedDelegateHandle = EquipmentManager->OnEquipmentEntryChanged.AddUObject(this, &ThisClass::OnEquipmentEntryChanged);
	
	EEquipState CurrentEquipState = EquipManager->GetCurrentEquipState();
	OnEquipStateChanged(CurrentEquipState, CurrentEquipState);
	EquipStateChangedDelegateHandle = EquipManager->OnEquipStateChanged.AddUObject(this, &ThisClass::OnEquipStateChanged);
}

void UPanUtilitySlotWidget::NativeDestruct()
{
	if (EquipmentManager)
	{
		EquipmentManager->OnEquipmentEntryChanged.Remove(EntryChangedDelegateHandle);
		EntryChangedDelegateHandle.Reset();
	}

	if (EquipManager)
	{
		EquipManager->OnEquipStateChanged.Remove(EquipStateChangedDelegateHandle);
		EquipStateChangedDelegateHandle.Reset();
	}
	
	Super::NativeDestruct();
}

void UPanUtilitySlotWidget::OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance, int32 ItemCount)
{
	EUtilitySlotType EntryUtilitySlotType = UPanEquipManagerComponent::ConvertToUtilitySlotType(EquipmentSlotType);
	if (EntryUtilitySlotType != WidgetUtilitySlotType)
		return;
	
	if (ItemInstance)
	{
		const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());

		Image_Icon->SetBrushFromTexture(ItemTemplate.IconTexture, true);
		Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (ItemCount > 1)
		{
			Text_Count->SetText(FText::AsNumber(ItemCount));
			Text_Count->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Text_Count->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		Image_Icon->SetVisibility(ESlateVisibility::Hidden);
		Text_Count->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPanUtilitySlotWidget::OnEquipStateChanged(EEquipState PrevEquipState, EEquipState NewEquipState)
{
	EEquipState SlotEquipState = UPanEquipManagerComponent::ConvertToEquipState(WidgetUtilitySlotType);

	if (NewEquipState == SlotEquipState)
	{
		PlayAnimationForward(Animation_Highlight_In);
	}
	else if (PrevEquipState == SlotEquipState)
	{
		PlayAnimationReverse(Animation_Highlight_In);
	}
}
