#include "Data/PanCharacterData.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

// �ּ� �Ŵ������� ĳ���� ������ ���
const UPanCharacterData& UPanCharacterData::Get()
{
	return ULyraAssetManager::Get().GetCharacterData();
}

#if WITH_EDITOR
// �ڵ� ���� ���� DefaultArmorMeshMap�� �Ǻλ� �̸��� ����
void UPanCharacterData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	DefaultArmorMeshMap.KeySort([](const ECharacterSkinType A, const ECharacterSkinType B)
		{
			return (A < B);
		});
}
#endif // WITH_EDITOR

// �Ǻλ� Ÿ���� ���� [�⺻ ��ü ���� ��Ʈ] ���
const FPanDefaultArmorMeshSet& UPanCharacterData::GetDefaultArmorMeshSet(ECharacterSkinType CharacterSkinType) const
{
	if (DefaultArmorMeshMap.Contains(CharacterSkinType) == false)
	{
		static FPanDefaultArmorMeshSet EmptyEntry;
		return EmptyEntry;
	}

	return DefaultArmorMeshMap.FindChecked(CharacterSkinType);
}
