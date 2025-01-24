#include "Data/PanCharacterData.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

// 애셋 매니저에서 캐릭터 데이터 얻기
const UPanCharacterData& UPanCharacterData::Get()
{
	return ULyraAssetManager::Get().GetCharacterData();
}

#if WITH_EDITOR
// 자동 저장 전에 DefaultArmorMeshMap의 피부색 이름순 정렬
void UPanCharacterData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	DefaultArmorMeshMap.KeySort([](const ECharacterSkinType A, const ECharacterSkinType B)
		{
			return (A < B);
		});
}
#endif // WITH_EDITOR

// 피부색 타입을 통해 [기본 신체 부위 세트] 얻기
const FPanDefaultArmorMeshSet& UPanCharacterData::GetDefaultArmorMeshSet(ECharacterSkinType CharacterSkinType) const
{
	if (DefaultArmorMeshMap.Contains(CharacterSkinType) == false)
	{
		static FPanDefaultArmorMeshSet EmptyEntry;
		return EmptyEntry;
	}

	return DefaultArmorMeshMap.FindChecked(CharacterSkinType);
}
