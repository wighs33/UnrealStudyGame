#include "PanClassData.h"

#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanClassData)

// 애셋매니저를 통해 자신을 얻기
const UPanClassData& UPanClassData::Get()
{
	return ULyraAssetManager::Get().GetClassData();
}

// 직업 타입을 통해 직업 정보 얻기
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
