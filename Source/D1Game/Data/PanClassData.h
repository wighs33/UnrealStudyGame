#pragma once

#include "PanDefine.h"
#include "PanClassData.generated.h"

class UPanItemTemplate;
class ULyraAbilitySet;

// 사용할 기본 아이템
USTRUCT(BlueprintType)
struct FPanDefaultItemEntry
{
	GENERATED_BODY()

public:
	// 장비 슬롯 타입
	UPROPERTY(EditDefaultsOnly)
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	// 아이템 템플릿 클래스
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanItemTemplate> ItemTemplateClass;

	// 레이도
	UPROPERTY(EditDefaultsOnly)
	EItemRarity ItemRarity = EItemRarity::Poor;

	// 개수
	UPROPERTY(EditDefaultsOnly)
	int32 ItemCount = 1;
};

// 직업 정보
USTRUCT(BlueprintType)
struct FPanClassInfoEntry
{
	GENERATED_BODY()

public:
	// 직업 이름
	UPROPERTY(EditDefaultsOnly)
	FText ClassName;

	// 기본 아이템 모음
	UPROPERTY(EditDefaultsOnly)
	TArray<FPanDefaultItemEntry> DefaultItemEntries;
	
	// 어빌리티 세트
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULyraAbilitySet> ClassAbilitySet;
};

// 직업 데이터
UCLASS(BlueprintType, Const)
class UPanClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	static const UPanClassData& Get();

public:
	const FPanClassInfoEntry& GetClassInfoEntry(ECharacterClassType ClassType) const;
	
private:
	// 직업 정보 모음
	UPROPERTY(EditDefaultsOnly, meta=(ArraySizeNum="ECharacterClassType"))
	FPanClassInfoEntry ClassInfoEntries[(int32)ECharacterClassType::Count];
};
