#include "PanCheatEntryWidget.h"

#include "CommonTextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Item/PanItemTemplate.h"
#include "Player/LyraCheatManager.h"
#include "Player/LyraPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanCheatEntryWidget)

UPanCheatEntryWidget::UPanCheatEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanCheatEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Entry->OnClicked.AddUniqueDynamic(this, &ThisClass::OnButtonClicked);
}

void UPanCheatEntryWidget::InitializeUI(EPanCheatEntryType InCheatEntryType, TSubclassOf<UPanItemTemplate> InItemTemplateClass, TSoftObjectPtr<UAnimMontage> InAnimMontage)
{
	CheatEntryType = InCheatEntryType;
	
	switch (CheatEntryType)
	{
	case EPanCheatEntryType::PrimaryWeapon:
	case EPanCheatEntryType::SecondaryWeapon:
	case EPanCheatEntryType::Armor:
	case EPanCheatEntryType::Utility:
		ItemTemplateClass = InItemTemplateClass;
		if (ItemTemplateClass)
		{
			if (UPanItemTemplate* ItemTemplate = Cast<UPanItemTemplate>(ItemTemplateClass->GetDefaultObject()))
			{
				Text_Entry->SetText(ItemTemplate->DisplayName);
				Image_Entry->SetBrushFromTexture(ItemTemplate->IconTexture, true);
			}
		}
		break;
	case EPanCheatEntryType::Animation:
		AnimMontage = InAnimMontage;
		if (AnimMontage.IsNull() == false)
		{
			Text_Entry->SetText(FText::FromString(AnimMontage.GetAssetName()));
			SizeBox_Entry->SetVisibility(ESlateVisibility::Collapsed);
		}
		break;
	}
}

void UPanCheatEntryWidget::OnButtonClicked()
{
	ALyraPlayerController* LyraPlayerController = GetOwningPlayer<ALyraPlayerController>();
	if (LyraPlayerController == nullptr)
		return;

	switch (CheatEntryType)
	{
	case EPanCheatEntryType::PrimaryWeapon:
		// TEMP Rookiss
		//LyraPlayerController->Server_EquipWeapon(EWeaponSlotType::Primary, ItemTemplateClass);
		break;
	case EPanCheatEntryType::SecondaryWeapon:
		// TEMP Rookiss
		//LyraPlayerController->Server_EquipWeapon(EWeaponSlotType::Secondary, ItemTemplateClass);
		break;
	case EPanCheatEntryType::Armor:
		// TEMP Rookiss
		//LyraPlayerController->Server_EquipArmor(ItemTemplateClass);
		break;
	case EPanCheatEntryType::Utility:
		// TEMP Rookiss
		//LyraPlayerController->Server_EquipUtility(EUtilitySlotType::Primary, ItemTemplateClass);
		break;
	case EPanCheatEntryType::Animation:
		if (ULyraCheatManager* LyraCheatManager = Cast<ULyraCheatManager>(LyraPlayerController->CheatManager))
		{
			LyraCheatManager->SelectedMontage = AnimMontage;
		}
		break;
	}
}
