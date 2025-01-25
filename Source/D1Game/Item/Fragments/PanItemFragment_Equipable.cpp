#include "PanItemFragment_Equipable.h"

#include "Item/PanItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemFragment_Equipable)

// 생성자
FRarityStatSet::FRarityStatSet()
{
	RarityStats.SetNum((int32)EItemRarity::Count);
	for (int32 i = 0; i < RarityStats.Num(); i++)
	{
		RarityStats[i].Rarity = (EItemRarity)i;
	}
}

// 생성자
FRarityStatRangeSet::FRarityStatRangeSet()
{
	RarityStatRanges.SetNum((int32)EItemRarity::Count);
	for (int32 i = 0; i < RarityStatRanges.Num(); i++)
	{
		RarityStatRanges[i].Rarity = (EItemRarity)i;
	}
}

// 생성자
UPanItemFragment_Equipable::UPanItemFragment_Equipable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

// 스탯을 통해 스택 추가
void UPanItemFragment_Equipable::AddStatTagStack(UPanItemInstance* ItemInstance, const TArray<FRarityStatSet>& RarityStatSets) const
{
	if (ItemInstance == nullptr)
		return;

	for (const FRarityStatSet& RarityStatSet : RarityStatSets)
	{
		const FGameplayTag& StatTag = RarityStatSet.StatTag;
		const FRarityStat& Stat = RarityStatSet.RarityStats[(int32)ItemInstance->GetItemRarity()];
		// 아이템 인스턴스에서 스탯과 태그로 스택 추가
		ItemInstance->AddStatTagStack(StatTag, Stat.Value);
	}
}

// 스탯 범위를 통해 스택 추가
void UPanItemFragment_Equipable::AddStatTagStack(UPanItemInstance* ItemInstance, const TArray<FRarityStatRangeSet>& RarityStatRangeSets) const
{
	if (ItemInstance == nullptr)
		return;
	
	for (const FRarityStatRangeSet& RarityStatRangeSet : RarityStatRangeSets)
	{
		const FGameplayTag& StatTag = RarityStatRangeSet.StatTag;
		const FRarityStatRange& StatRange = RarityStatRangeSet.RarityStatRanges[(int32)ItemInstance->GetItemRarity()];
		const int32 StatValue = FMath::RandRange(StatRange.MinValue, StatRange.MaxValue);
		ItemInstance->AddStatTagStack(StatTag, StatValue);
	}
}

// 착용가능 클래스 타입인지
bool UPanItemFragment_Equipable::IsEquipableClassType(ECharacterClassType ClassType) const
{
	return (EquipableClassFlags & (1 << (uint32)ClassType)) != 0;
}
