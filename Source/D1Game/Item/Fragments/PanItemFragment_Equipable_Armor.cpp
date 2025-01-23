#include "PanItemFragment_Equipable_Armor.h"

#include "Item/PanItemInstance.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemFragment_Equipable_Armor)

UPanItemFragment_Equipable_Armor::UPanItemFragment_Equipable_Armor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    EquipmentType = EEquipmentType::Armor;
}

#if WITH_EDITORONLY_DATA
void UPanItemFragment_Equipable_Armor::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	if (ArmorType != EArmorType::Chest)
	{
		bIsFullBody = false;
	}
	
	for (int i = 0; i < RarityStatRangeSets.Num(); i++)
	{
		TArray<FRarityStatRange>& RarityStatRanges = RarityStatRangeSets[i].RarityStatRanges;
		RarityStatRanges.SetNum((int32)EItemRarity::Count);
		for (int32 j = 0; j < RarityStatRanges.Num(); j++)
		{
			RarityStatRanges[j].Rarity = (EItemRarity)j;
		}
	}
}
#endif // WITH_EDITORONLY_DATA

void UPanItemFragment_Equipable_Armor::OnInstanceCreated(UPanItemInstance* ItemInstance) const
{
	Super::OnInstanceCreated(ItemInstance);
	
	AddStatTagStack(ItemInstance, RarityStatRangeSets);
}
