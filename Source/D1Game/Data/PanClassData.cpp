#include "PanClassData.h"

#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanClassData)

const UPanClassData& UPanClassData::Get()
{
	return ULyraAssetManager::Get().GetClassData();
}

const FPanClassInfoEntry& UPanClassData::GetClassInfoEntry(ECharacterClassType ClassType) const
{
	const int32 ClassIndex = (int32)ClassType;
	if (ClassIndex < 0 || ClassIndex >= (int32)ECharacterClassType::Count)
	{
		static FPanClassInfoEntry EmptyEntry;
		return EmptyEntry;
	}
	
	return ClassInfoEntries[ClassIndex];
}
