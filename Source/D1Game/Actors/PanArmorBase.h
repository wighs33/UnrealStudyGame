#pragma once

#include "PanArmorBase.generated.h"

// 갑옷 부위 액터
UCLASS(BlueprintType, Blueprintable)
class APanArmorBase : public AActor
{
	GENERATED_BODY()
	
public:
	APanArmorBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void InitializeActor(TSoftObjectPtr<USkeletalMesh> InDefaultArmorMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryArmorMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial);

public:
	void SetPrimaryArmorMesh(USkeletalMesh* InPrimaryArmorMesh);
	void ShouldUseSecondaryMesh(bool bInShouldUseSecondary);

private:
	void RefreshArmorMesh();
	void SetArmorMesh(TSoftObjectPtr<USkeletalMesh> InArmorMesh);
	
public:
	USkeletalMeshComponent* GetMeshComponent() const { return ArmorMeshComponent; }
	
protected:
	// 스켈레탈 메시 컴포넌트 : 갑옷 부위
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> ArmorMeshComponent;

protected:
	// 기본 갑옷
	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh> DefaultArmorMesh;
	// 주요 갑옷
	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh> PrimaryArmorMesh;
	// 보조 갑옷
	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh> SecondaryArmorMesh;

protected:
	// 피부 머티리얼 슬롯 이름
	UPROPERTY()
	FName SkinMaterialSlotName;

	// 피부 머티리얼
	UPROPERTY()
	TSoftObjectPtr<UMaterialInterface> SkinMaterial;
	
private:
	// 보조 갑옷 사용가능성
	bool bShouldUseSecondaryArmor = false;
};
