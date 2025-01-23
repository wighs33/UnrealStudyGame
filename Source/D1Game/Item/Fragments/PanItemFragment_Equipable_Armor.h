#pragma once

#include "PanDefine.h"
#include "PanItemFragment_Equipable.h"
#include "PanItemFragment_Equipable_Armor.generated.h"

UCLASS()
class UPanItemFragment_Equipable_Armor : public UPanItemFragment_Equipable
{
	GENERATED_BODY()
	
public:
	UPanItemFragment_Equipable_Armor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif // WITH_EDITORONLY_DATA

public:
	virtual void OnInstanceCreated(UPanItemInstance* ItemInstance) const override;
	
public:
	UPROPERTY(EditDefaultsOnly)
	EArmorType ArmorType = EArmorType::Count;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ArmorType == EArmorType::Chest", EditConditionHides))
	bool bIsFullBody = false;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> ArmorMesh;

	UPROPERTY(EditDefaultsOnly, meta=(ForceInlineRow))
	TArray<FRarityStatRangeSet> RarityStatRangeSets;
};
