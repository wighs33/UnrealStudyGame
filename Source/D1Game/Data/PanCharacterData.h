#pragma once

#include "PanDefine.h"
#include "Engine/DataAsset.h"
#include "PanCharacterData.generated.h"

// 기본 신체 부위 세트
USTRUCT(BlueprintType)
struct FPanDefaultArmorMeshSet
{
	GENERATED_BODY()
		
public:
	// 상체
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMaterialInterface> UpperBodySkinMaterial;
	
	// 하체
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMaterialInterface> LowerBodySkinMaterial;
	
public:
	// 기본 머리
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> HeadDefaultMesh;
	// 보조 머리
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> HeadSecondaryMesh;

public:
	// 기본 부위들
	UPROPERTY(EditDefaultsOnly, meta=(ArraySizeEnum="EArmorType"))
	TSoftObjectPtr<USkeletalMesh> DefaultMeshEntries[(int32)EArmorType::Count];

	// 보조 부위들
	UPROPERTY(EditDefaultsOnly, meta=(ArraySizeEnum="EArmorType"))
	TSoftObjectPtr<USkeletalMesh> SecondaryMeshEntries[(int32)EArmorType::Count];
};

// 캐릭터 데이터 (싱글턴)
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
	// 피부색 - 기본 신체 부위 세트
	UPROPERTY(EditDefaultsOnly)
	TMap<ECharacterSkinType, FPanDefaultArmorMeshSet> DefaultArmorMeshMap;
};
