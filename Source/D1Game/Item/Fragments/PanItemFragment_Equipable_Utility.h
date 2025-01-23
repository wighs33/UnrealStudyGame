#pragma once

#include "PanItemFragment_Equipable_Attachment.h"
#include "PanItemFragment_Equipable_Utility.generated.h"

class UGameplayEffect;

UCLASS()
class UPanItemFragment_Equipable_Utility : public UPanItemFragment_Equipable_Attachment
{
	GENERATED_BODY()

public:
	UPanItemFragment_Equipable_Utility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif // WITH_EDITORONLY_DATA

public:
	virtual void OnInstanceCreated(UPanItemInstance* ItemInstance) const override;

public:
	UPROPERTY(EditDefaultsOnly)
	EUtilityType UtilityType = EUtilityType::Count;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> UtilityEffectClass;
	
	UPROPERTY(EditDefaultsOnly, meta=(ForceInlineRow))
	TArray<FRarityStatSet> RarityStatSets;
};
