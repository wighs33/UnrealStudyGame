#pragma once

#include "PanDefine.h"
#include "PanClassData.generated.h"

class UPanItemTemplate;
class ULyraAbilitySet;

USTRUCT(BlueprintType)
struct FPanDefaultItemEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanItemTemplate> ItemTemplateClass;

	UPROPERTY(EditDefaultsOnly)
	EItemRarity ItemRarity = EItemRarity::Poor;

	UPROPERTY(EditDefaultsOnly)
	int32 ItemCount = 1;
};

USTRUCT(BlueprintType)
struct FPanClassInfoEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FText ClassName;

	UPROPERTY(EditDefaultsOnly)
	TArray<FPanDefaultItemEntry> DefaultItemEntries;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULyraAbilitySet> ClassAbilitySet;
};

UCLASS(BlueprintType, Const)
class UPanClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	static const UPanClassData& Get();

public:
	const FPanClassInfoEntry& GetClassInfoEntry(ECharacterClassType ClassType) const;
	
private:
	UPROPERTY(EditDefaultsOnly, meta=(ArraySizeNum="ECharacterClassType"))
	FPanClassInfoEntry ClassInfoEntries[(int32)ECharacterClassType::Count];
};
