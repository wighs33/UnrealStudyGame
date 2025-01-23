// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/D1CharacterData.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

const UD1CharacterData& UD1CharacterData::Get()
{
	return ULyraAssetManager::Get().GetCharacterData();
}

#if WITH_EDITOR
void UD1CharacterData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	DefaultArmorMeshMap.KeySort([](const ECharacterSkinType A, const ECharacterSkinType B)
		{
			return (A < B);
		});
}
#endif // WITH_EDITOR

const FD1DefaultArmorMeshSet& UD1CharacterData::GetDefaultArmorMeshSet(ECharacterSkinType CharacterSkinType) const
{
	if (DefaultArmorMeshMap.Contains(CharacterSkinType) == false)
	{
		static FD1DefaultArmorMeshSet EmptyEntry;
		return EmptyEntry;
	}

	return DefaultArmorMeshMap.FindChecked(CharacterSkinType);
}
