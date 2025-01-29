#include "PanUIData.h"

#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanUIData)

const UPanUIData& UPanUIData::Get()
{
	return ULyraAssetManager::Get().GetUIData();
}

UTexture2D* UPanUIData::GetEntryRarityTexture(EItemRarity ItemRarity) const
{
	const int32 ItemRarityIndex = (int32)ItemRarity;
	if (ItemRarityIndex < 0 || ItemRarityIndex >= (int32)EItemRarity::Count)
		return nullptr;
	
	return RarityInfoEntries[ItemRarityIndex].EntryTexture;
}

UTexture2D* UPanUIData::GetHoverRarityTexture(EItemRarity ItemRarity) const
{
	const int32 ItemRarityIndex = (int32)ItemRarity;
	if (ItemRarityIndex < 0 || ItemRarityIndex >= (int32)EItemRarity::Count)
		return nullptr;
	
	return RarityInfoEntries[ItemRarityIndex].HoverTexture;
}

FColor UPanUIData::GetRarityColor(EItemRarity ItemRarity) const
{
	const int32 ItemRarityIndex = (int32)ItemRarity;
	if (ItemRarityIndex < 0 || ItemRarityIndex >= (int32)EItemRarity::Count)
		return FColor::Black;
	
	return RarityInfoEntries[ItemRarityIndex].Color;
}

const FPanUIInfo& UPanUIData::GetTagUIInfo(FGameplayTag Tag) const
{
	const FPanUIInfo* UIInfo = TagUIInfos.Find(Tag);
	if (UIInfo == nullptr)
	{
		static FPanUIInfo EmptyInfo;
		return EmptyInfo;
	}

	return *UIInfo;
}
