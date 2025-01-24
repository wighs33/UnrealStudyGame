#pragma once

#include "PanDefine.h"
#include "Engine/DataAsset.h"
#include "PanCharacterData.generated.h"

// �⺻ ��ü ���� ��Ʈ
USTRUCT(BlueprintType)
struct FPanDefaultArmorMeshSet
{
	GENERATED_BODY()
		
public:
	// ��ü
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMaterialInterface> UpperBodySkinMaterial;
	
	// ��ü
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMaterialInterface> LowerBodySkinMaterial;
	
public:
	// �⺻ �Ӹ�
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> HeadDefaultMesh;
	// ���� �Ӹ�
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> HeadSecondaryMesh;

public:
	// �⺻ ������
	UPROPERTY(EditDefaultsOnly, meta=(ArraySizeEnum="EArmorType"))
	TSoftObjectPtr<USkeletalMesh> DefaultMeshEntries[(int32)EArmorType::Count];

	// ���� ������
	UPROPERTY(EditDefaultsOnly, meta=(ArraySizeEnum="EArmorType"))
	TSoftObjectPtr<USkeletalMesh> SecondaryMeshEntries[(int32)EArmorType::Count];
};

// ĳ���� ������ (�̱���)
UCLASS(BlueprintType)
class D1GAME_API UPanCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	static const UPanCharacterData& Get();

public:
#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
#endif // WITH_EDITOR

public:
	const FPanDefaultArmorMeshSet& GetDefaultArmorMeshSet(ECharacterSkinType CharacterSkinType) const;

private:
	// �Ǻλ� - �⺻ ��ü ���� ��Ʈ
	UPROPERTY(EditDefaultsOnly)
	TMap<ECharacterSkinType, FPanDefaultArmorMeshSet> DefaultArmorMeshMap;
};
