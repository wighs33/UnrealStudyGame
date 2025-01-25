#pragma once

#include "PanDefine.h"
#include "GameplayTags.h"
#include "Item/PanItemTemplate.h"
#include "PanItemFragment_Equipable.generated.h"

class ULyraAbilitySet;
class UPanItemInstance;

// 레어 스탯
USTRUCT(BlueprintType)
struct FRarityStat
{
	GENERATED_BODY()

public:
	// 레어도
	UPROPERTY(VisibleDefaultsOnly)
	EItemRarity Rarity = EItemRarity::Poor;

	// 레어 수치
	UPROPERTY(EditDefaultsOnly)
	int32 Value = 0;
};

// 레어 스탯 모음
USTRUCT(BlueprintType)
struct FRarityStatSet
{
	GENERATED_BODY()

public:
	FRarityStatSet();
	
public:
	// 스탯 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="SetByCaller"))
	FGameplayTag StatTag;
	
	// 레어도 스탯 모음
	UPROPERTY(EditDefaultsOnly, EditFixedSize)
	TArray<FRarityStat> RarityStats;
};

// 레어도 스탯 범위
USTRUCT(BlueprintType)
struct FRarityStatRange
{
	GENERATED_BODY()

public:
	// 레어도
	UPROPERTY(VisibleDefaultsOnly)
	EItemRarity Rarity = EItemRarity::Poor;
	
	// 최솟값
	UPROPERTY(EditDefaultsOnly)
	int32 MinValue = 0;

	// 최댓값
	UPROPERTY(EditDefaultsOnly)
	int32 MaxValue = 0;
};

// 레어도 스탯 범위 모음
USTRUCT(BlueprintType)
struct FRarityStatRangeSet
{
	GENERATED_BODY()

public:
	FRarityStatRangeSet();
	
public:
	// 스탯 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="SetByCaller"))
	FGameplayTag StatTag;
	
	// 레어도 스탯 범위 모음
	UPROPERTY(EditDefaultsOnly, EditFixedSize)
	TArray<FRarityStatRange> RarityStatRanges;
};

// 착용가능한 프래그먼트
UCLASS(Abstract, Const)
class UPanItemFragment_Equipable : public UPanItemFragment
{
	GENERATED_BODY()
	
public:
	UPanItemFragment_Equipable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	void AddStatTagStack(UPanItemInstance* ItemInstance, const TArray<FRarityStatSet>& RarityStatSets) const;
	void AddStatTagStack(UPanItemInstance* ItemInstance, const TArray<FRarityStatRangeSet>& RarityStatRangeSets) const;

public:
	bool IsEquipableClassType(ECharacterClassType ClassType) const;
	
public:
	// 장비 타입
	EEquipmentType EquipmentType = EEquipmentType::Count;
	
	// 캐릭터 직업 개수는 5이므로 11111
	UPROPERTY(EditDefaultsOnly, meta=(Bitmask, BitmaskEnum="ECharacterClassType"))
	uint32 EquipableClassFlags = ((1 << (uint32)ECharacterClassType::Count) - 1);
	
	// 기본 어빌리티 세트
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const ULyraAbilitySet> BaseAbilitySet;
};
