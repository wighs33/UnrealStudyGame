﻿#pragma once

#include "PanDefine.h"
#include "AbilitySystem/LyraAbilitySourceInterface.h"
#include "System/PanGameplayTagStack.h"
#include "PanItemInstance.generated.h"

// 아이템 레어도 별 등장 확률
USTRUCT(BlueprintType)
struct FPanItemRarityProbability
{
	GENERATED_BODY()

public:
	// 레어도
	UPROPERTY(VisibleAnywhere)
	EItemRarity Rarity = EItemRarity::Poor;

	// 등장 확률
	UPROPERTY(EditAnywhere)
	float Probability = 0;
};

// 아이템 정보
UCLASS(BlueprintType)
class UPanItemInstance : public UObject, public ILyraAbilitySourceInterface
{
	GENERATED_BODY()
	
public:
	UPanItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	
public:
	void Init(int32 InItemTemplateID, EItemRarity InItemRarity);

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddStatTagStack(FGameplayTag StatTag, int32 StackCount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemoveStatTagStack(FGameplayTag StatTag);

public:
	static EItemRarity DetermineItemRarity(const TArray<FPanItemRarityProbability>& ItemProbabilities);
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetItemTemplateID() const { return ItemTemplateID; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	EItemRarity GetItemRarity() const { return ItemRarity; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasStatTag(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetStackCountByTag(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FPanGameplayTagStackContainer& GetStatContainer() const { return StatContainer; }

public:
	UFUNCTION(BlueprintCallable, BlueprintPure="false", meta=(DeterminesOutputType="FragmentClass"))
	const UPanItemFragment* FindFragmentByClass(TSubclassOf<UPanItemFragment> FragmentClass) const;

	template <typename FragmentClass>
	const FragmentClass* FindFragmentByClass() const
	{
		return (FragmentClass*)FindFragmentByClass(FragmentClass::StaticClass());
	}
	
private:
#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS
	
private:
	// 아이템 템플릿 아이디
	UPROPERTY(Replicated)
	int32 ItemTemplateID = INDEX_NONE;

	// 아이템 레어도
	UPROPERTY(Replicated)
	EItemRarity ItemRarity = EItemRarity::Poor;
	
	// 스탯 컨테이너
	UPROPERTY(Replicated)
	FPanGameplayTagStackContainer StatContainer;
};
