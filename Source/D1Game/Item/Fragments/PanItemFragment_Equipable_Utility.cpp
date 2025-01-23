﻿#include "PanItemFragment_Equipable_Utility.h"

#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemFragment_Equipable_Utility)

UPanItemFragment_Equipable_Utility::UPanItemFragment_Equipable_Utility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    EquipmentType = EEquipmentType::Utility;
}

#if WITH_EDITORONLY_DATA
void UPanItemFragment_Equipable_Utility::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	for (int i = 0; i < RarityStatSets.Num(); i++)
	{
		TArray<FRarityStat>& RarityStats = RarityStatSets[i].RarityStats;
		RarityStats.SetNum((int32)EItemRarity::Count);
		for (int32 j = 0; j < RarityStats.Num(); j++)
		{
			RarityStats[j].Rarity = (EItemRarity)j;
		}
	}
}
#endif // WITH_EDITORONLY_DATA

void UPanItemFragment_Equipable_Utility::OnInstanceCreated(UPanItemInstance* ItemInstance) const
{
	Super::OnInstanceCreated(ItemInstance);
	
	AddStatTagStack(ItemInstance, RarityStatSets);
}
