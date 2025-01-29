#include "PanCheatData.h"

#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanCheatData)

const UPanCheatData& UPanCheatData::Get()
{
	return ULyraAssetManager::Get().GetCheatData();
}
