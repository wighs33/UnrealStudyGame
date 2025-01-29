#include "PanCheatMenuWidget.h"

#include "PanCheatEntryWidget.h"
#include "PanCheatListWidget.h"
#include "Components/Button.h"
#include "Data/PanCheatData.h"
#include "Data/PanItemData.h"
#include "Item/PanItemTemplate.h"
#include "Item/Fragments/PanItemFragment_Equipable.h"
#include "Item/Fragments/PanItemFragment_Equipable_Weapon.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanCheatMenuWidget)

UPanCheatMenuWidget::UPanCheatMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanCheatMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CheatEntryWidgetClass = ULyraAssetManager::GetSubclassByName<UPanCheatEntryWidget>("CheatEntryWidgetClass");
	Button_Exit->OnClicked.AddUniqueDynamic(this, &ThisClass::OnExitButtonClicked);
}

void UPanCheatMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UPanItemData& ItemData = ULyraAssetManager::Get().GetItemData();

	TArray<TSubclassOf<UPanItemTemplate>> ItemTemplateClasses;
	ItemData.GetAllItemTemplateClasses(ItemTemplateClasses);

	for (TSubclassOf<UPanItemTemplate> ItemTemplateClass : ItemTemplateClasses)
	{
		const UPanItemTemplate* ItemTemplate = ItemTemplateClass.GetDefaultObject();
		if (const UPanItemFragment_Equipable* EquippableFragment = ItemTemplate->FindFragmentByClass<UPanItemFragment_Equipable>())
		{
			if (EquippableFragment->EquipmentType == EEquipmentType::Weapon)
			{
				const UPanItemFragment_Equipable_Weapon* WeaponFragment = ItemTemplate->FindFragmentByClass<UPanItemFragment_Equipable_Weapon>();
				if (WeaponFragment->WeaponType == EWeaponType::Unarmed)
					continue;
				
				UPanCheatEntryWidget* CheatEntryWidget = CreateWidget<UPanCheatEntryWidget>(GetOwningPlayer(), CheatEntryWidgetClass);
				CheatEntryWidget->InitializeUI(EPanCheatEntryType::PrimaryWeapon, ItemTemplateClass, nullptr);
				CheatList_PrimaryWeapon->AddEntry(CheatEntryWidget);

				CheatEntryWidget = CreateWidget<UPanCheatEntryWidget>(GetOwningPlayer(), CheatEntryWidgetClass);
				CheatEntryWidget->InitializeUI(EPanCheatEntryType::SecondaryWeapon, ItemTemplateClass, nullptr);
				CheatList_SecondaryWeapon->AddEntry(CheatEntryWidget);
			}
			else if (EquippableFragment->EquipmentType == EEquipmentType::Utility)
			{
				UPanCheatEntryWidget* CheatEntryWidget = CreateWidget<UPanCheatEntryWidget>(GetOwningPlayer(), CheatEntryWidgetClass);
				CheatEntryWidget->InitializeUI(EPanCheatEntryType::Utility, ItemTemplateClass, nullptr);
				CheatList_Utility->AddEntry(CheatEntryWidget);
			}
			else if (EquippableFragment->EquipmentType == EEquipmentType::Armor)
			{
				UPanCheatEntryWidget* CheatEntryWidget = CreateWidget<UPanCheatEntryWidget>(GetOwningPlayer(), CheatEntryWidgetClass);
				CheatEntryWidget->InitializeUI(EPanCheatEntryType::Armor, ItemTemplateClass, nullptr);
				CheatList_Armor->AddEntry(CheatEntryWidget);
			}
		}
	}

	const UPanCheatData& CheatData = ULyraAssetManager::Get().GetCheatData();
	
	for (const TSoftObjectPtr<UAnimMontage>& AnimMontagePath : CheatData.GetAnimMontagePaths())
	{
		UPanCheatEntryWidget* CheatEntryWidget = CreateWidget<UPanCheatEntryWidget>(GetOwningPlayer(), CheatEntryWidgetClass);
		CheatEntryWidget->InitializeUI(EPanCheatEntryType::Animation, nullptr, AnimMontagePath);
		CheatList_Animation->AddEntry(CheatEntryWidget);
	}
}

void UPanCheatMenuWidget::OnExitButtonClicked()
{
	DeactivateWidget();
}
