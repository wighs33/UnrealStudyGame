#pragma once

#include "PanCheatData.generated.h"

class UGameplayEffect;

UCLASS(Const)
class UPanCheatData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	static const UPanCheatData& Get();

public:
	const TArray<TSoftObjectPtr<UAnimMontage>>& GetAnimMontagePaths() const { return AnimMontagePaths; }
	const TSubclassOf<UGameplayEffect> GetResetCooldownGameplayEffectClass() const { return ResetCooldownGameplayEffectClass; }
	const TSubclassOf<UGameplayEffect> GetResetVitalGameplayEffectClass() const { return ResetVitalGameplayEffectClass; }
	
private:
	UPROPERTY(EditDefaultsOnly)
	TArray<TSoftObjectPtr<UAnimMontage>> AnimMontagePaths;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> ResetCooldownGameplayEffectClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> ResetVitalGameplayEffectClass;
};
