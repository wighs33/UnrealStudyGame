#pragma once

#include "PanArmorBase.generated.h"

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
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> ArmorMeshComponent;

protected:
	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh> DefaultArmorMesh;
	
	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh> PrimaryArmorMesh;

	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh> SecondaryArmorMesh;

protected:
	UPROPERTY()
	FName SkinMaterialSlotName;

	UPROPERTY()
	TSoftObjectPtr<UMaterialInterface> SkinMaterial;
	
private:
	bool bShouldUseSecondaryArmor = false;
};
