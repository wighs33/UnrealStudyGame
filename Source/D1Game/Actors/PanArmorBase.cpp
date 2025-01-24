#include "PanArmorBase.h"

#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanArmorBase)

// 생성자
APanArmorBase::APanArmorBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// ArmorMeshComponent 컴포넌트 생성
    ArmorMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("ArmorMeshComponent");
	ArmorMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(ArmorMeshComponent);
}

// 프로퍼티에 초기값 채우고 [기본 갑옷]를 자신의 메시로 적용
void APanArmorBase::InitializeActor(TSoftObjectPtr<USkeletalMesh> InDefaultArmorMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryArmorMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial)
{
	DefaultArmorMesh = InDefaultArmorMesh;
	SecondaryArmorMesh = (InSecondaryArmorMesh.IsNull() == false ? InSecondaryArmorMesh : InDefaultArmorMesh);
	
	if (InSkinMaterialSlotName.IsNone() == false && InSkinMaterial.IsNull() == false)
	{
		SkinMaterialSlotName = InSkinMaterialSlotName;
		SkinMaterial = InSkinMaterial;
	}

	SetArmorMesh(DefaultArmorMesh);
}

// [주요 갑옷]을 갱신 후 리프레시
void APanArmorBase::SetPrimaryArmorMesh(USkeletalMesh* InPrimaryArmorMesh)
{
	PrimaryArmorMesh = InPrimaryArmorMesh;
	RefreshArmorMesh();
}

// [보조 갑옷 사용가능성]을 갱신 후 리프레시
void APanArmorBase::ShouldUseSecondaryMesh(bool bInShouldUseSecondary)
{
	bShouldUseSecondaryArmor = bInShouldUseSecondary;
	RefreshArmorMesh();
}

// [보조 갑옷 사용가능성]에 따라 자신의 메시를 올바르게 재설정
void APanArmorBase::RefreshArmorMesh()
{
	TSoftObjectPtr<USkeletalMesh> ArmorMesh;
	if (bShouldUseSecondaryArmor)
	{
		ArmorMesh = SecondaryArmorMesh;
	}
	else
	{
		ArmorMesh = PrimaryArmorMesh ? PrimaryArmorMesh : DefaultArmorMesh;
	}

	SetArmorMesh(ArmorMesh);
}

// 애셋 매니저에서 메시와 머티리얼을 로드 후 메시 컴포넌트에 설정
void APanArmorBase::SetArmorMesh(TSoftObjectPtr<USkeletalMesh> InArmorMesh)
{
	USkeletalMesh* LoadedArmorMesh = nullptr;
	if (InArmorMesh.IsNull() == false)
	{
		LoadedArmorMesh = ULyraAssetManager::GetAssetByPath<USkeletalMesh>(InArmorMesh);
	}
	
	ArmorMeshComponent->SetSkeletalMesh(LoadedArmorMesh);
	ArmorMeshComponent->EmptyOverrideMaterials();

	if (SkinMaterialSlotName.IsNone() == false && SkinMaterial.IsNull() == false)
	{
		UMaterialInterface* LoadedMaterial = ULyraAssetManager::GetAssetByPath<UMaterialInterface>(SkinMaterial);
		ArmorMeshComponent->SetMaterialByName(SkinMaterialSlotName, LoadedMaterial);
	}
}
