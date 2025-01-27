#pragma once

#include "PanDefine.h"
#include "Components/PawnComponent.h"
#include "PanCosmeticManagerComponent.generated.h"

class APanArmorBase;
class UPanItemFragment_Equipable_Armor;

// 캐릭터 겉치장 관리 컴포넌트
UCLASS(BlueprintType, Blueprintable)
class UPanCosmeticManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
	UPanCosmeticManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	void RefreshArmorMesh(EArmorType ArmorType , const UPanItemFragment_Equipable_Armor* ArmorFragment);
	void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const;

private:
	void InitializeManager();
	
	UChildActorComponent* SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial);
	void SetPrimaryArmorMesh(EArmorType ArmorType, TSoftObjectPtr<USkeletalMesh> ArmorMeshPtr);
	
protected:
	// 피부타입
	UPROPERTY(EditDefaultsOnly)
	ECharacterSkinType CharacterSkinType = ECharacterSkinType::Asian;
	
	// 치장 슬롯 리소스
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APanArmorBase> CosmeticSlotClass;

private:
	UPROPERTY()
	// 머리 슬롯
	TObjectPtr<UChildActorComponent> HeadSlot;
	
	// 치장 슬롯
	UPROPERTY()
	TArray<TObjectPtr<UChildActorComponent>> CosmeticSlots;

	bool bInitialized = false;
};
