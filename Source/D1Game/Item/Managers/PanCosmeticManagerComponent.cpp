#include "PanCosmeticManagerComponent.h"

#include "PanDefine.h"
#include "Actors/PanArmorBase.h"
#include "Character/LyraCharacter.h"
#include "Data/PanCharacterData.h"
#include "Item/Fragments/PanItemFragment_Equipable_Armor.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanCosmeticManagerComponent)

// 생성자
UPanCosmeticManagerComponent::UPanCosmeticManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 기본 리플리케이션
	SetIsReplicatedByDefault(true);
}

// 플레이 시작
void UPanCosmeticManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 매니저 초기화
	InitializeManager();
}

// 플레이 종료
void UPanCosmeticManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 머리 슬롯 제거
	if (HeadSlot)
	{
		HeadSlot->DestroyComponent();
	}
	
	// 치장 슬롯 제거
	for (UChildActorComponent* CosmeticSlot : CosmeticSlots)
	{
		if (CosmeticSlot)
		{
			CosmeticSlot->DestroyComponent();
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

// 리프레시
void UPanCosmeticManagerComponent::RefreshArmorMesh(EArmorType ArmorType, const UPanItemFragment_Equipable_Armor* ArmorFragment)
{
	if (ArmorType == EArmorType::Count)
		return;
	
	// 매니저 초기화
	InitializeManager();
	
	if (ArmorFragment)
	{
		if (ArmorFragment == nullptr || ArmorFragment->ArmorType != ArmorType)
			return;
		
		SetPrimaryArmorMesh(ArmorType, ArmorFragment->ArmorMesh);
		
		if (ArmorType == EArmorType::Chest)
		{
			if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)EArmorType::Legs])
			{
				if (APanArmorBase* CosmeticActor = Cast<APanArmorBase>(CosmeticSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(ArmorFragment->bIsFullBody);
				}
			}
		}
		else if (ArmorType == EArmorType::Helmet)
		{
			if (HeadSlot)
			{
				if (APanArmorBase* CosmeticActor = Cast<APanArmorBase>(HeadSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(true);
				}
			}
		}
	}
	else
	{
		// 가슴 부위일 때 [보조 메시 사용 가능성] 초기화
		if (ArmorType == EArmorType::Chest)
		{
			if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)EArmorType::Legs])
			{
				if (APanArmorBase* CosmeticActor = Cast<APanArmorBase>(CosmeticSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(false);
				}
			}
		}
		// 머리 부위일 때 [보조 메시 사용 가능성] 초기화
		else if (ArmorType == EArmorType::Helmet)
		{
			if (HeadSlot)
			{
				if (APanArmorBase* CosmeticActor = Cast<APanArmorBase>(HeadSlot->GetChildActor()))
				{
					CosmeticActor->ShouldUseSecondaryMesh(false);
				}
			}
		}

		// 해당 부위의 주요 갑옷 초기화
		SetPrimaryArmorMesh(ArmorType, nullptr);
	}
}

// 해당 부위의 주요 갑옷 세팅
void UPanCosmeticManagerComponent::SetPrimaryArmorMesh(EArmorType ArmorType, TSoftObjectPtr<USkeletalMesh> ArmorMeshPtr)
{
	if (ArmorType == EArmorType::Count)
		return;
	
	// 매니저 초기화
	InitializeManager();
	
	// 해당 부위 슬롯의 유효성 체크
	if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)ArmorType])
	{
		if (APanArmorBase* CosmeticActor = Cast<APanArmorBase>(CosmeticSlot->GetChildActor()))
		{
			if (ArmorMeshPtr.IsNull())
			{
				CosmeticActor->SetPrimaryArmorMesh(nullptr);
			}
			else
			{
				// 애셋 매니저에서 갑옷 로드
				USkeletalMesh* ArmorMesh = ULyraAssetManager::GetAssetByPath<USkeletalMesh>(ArmorMeshPtr);
				// 주요 갑옷 세팅
				CosmeticActor->SetPrimaryArmorMesh(ArmorMesh);
			}
		}
	}
}

// 메시 컴포넌트 모음 얻기
void UPanCosmeticManagerComponent::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	// 머리 슬롯의 액터를 메시 컴포넌트 모음에 추가
	if (HeadSlot)
	{
		if (APanArmorBase* CosmeticActor = Cast<APanArmorBase>(HeadSlot->GetChildActor()))
		{
			OutMeshComponents.Add(CosmeticActor->GetMeshComponent());
		}
	}
	
	// 치장 슬롯의 액터를 메시 컴포넌트 모음에 추가
	for (UChildActorComponent* CosmeticSlot : CosmeticSlots)
	{
		if (CosmeticSlot)
		{
			if (APanArmorBase* CosmeticActor = Cast<APanArmorBase>(CosmeticSlot->GetChildActor()))
			{
				OutMeshComponents.Add(CosmeticActor->GetMeshComponent());
			}
		}
	}
}

// 매니저 초기화
void UPanCosmeticManagerComponent::InitializeManager()
{
	if (bInitialized)
		return;

	bInitialized = true;

	// 치장 슬롯 그룹의 수 리사이즈
	const int32 ArmorTypeCount = (int32)EArmorType::Count;
	CosmeticSlots.SetNumZeroed(ArmorTypeCount);
	
	// 유효성체크
	check(CosmeticSlotClass);
	check(CharacterSkinType != ECharacterSkinType::Count);
	
	//
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		// 데디서버가 아닐 때
		if (Character->IsNetMode(NM_DedicatedServer) == false)
		{
			const UPanCharacterData& CharacterData = ULyraAssetManager::Get().GetCharacterData();
			const FPanDefaultArmorMeshSet& DefaultArmorMeshSet = CharacterData.GetDefaultArmorMeshSet(CharacterSkinType);
			
			// 기본 부위 세트의 머리 메시를 통해 머리 슬롯 생성
			HeadSlot = SpawnCosmeticSlotActor(DefaultArmorMeshSet.HeadDefaultMesh, DefaultArmorMeshSet.HeadSecondaryMesh, NAME_None, nullptr);
			
			// 기본 부위 세트의 모든 부위 메시를 통해 모든 부위의 슬롯 생성
			for (int32 i = 0; i < (int32)EArmorType::Count; i++)
			{
				EArmorType ArmorType = (EArmorType)i;
				FName SkinMaterialSlotName;
				TSoftObjectPtr<UMaterialInterface> SkinMaterial;
				
				// 상체 머티리얼
				if (ArmorType == EArmorType::Helmet || ArmorType == EArmorType::Chest || ArmorType == EArmorType::Hands)
				{
					SkinMaterialSlotName = FName("UpperBody");
					SkinMaterial = DefaultArmorMeshSet.UpperBodySkinMaterial;
				}
				// 하체 머티리얼
				else if (ArmorType == EArmorType::Legs || ArmorType == EArmorType::Foot)
				{
					SkinMaterialSlotName = FName("LowerBody");
					SkinMaterial = DefaultArmorMeshSet.LowerBodySkinMaterial;
				}
				
				CosmeticSlots[i] = SpawnCosmeticSlotActor(DefaultArmorMeshSet.DefaultMeshEntries[i], DefaultArmorMeshSet.SecondaryMeshEntries[i], SkinMaterialSlotName, SkinMaterial);
			}
		}
	}
}

// 해당 부위 메시를 통해 해당 부위의 슬롯 생성
UChildActorComponent* UPanCosmeticManagerComponent::SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh, TSoftObjectPtr<USkeletalMesh> InSecondaryMesh, FName InSkinMaterialSlotName, TSoftObjectPtr<UMaterialInterface> InSkinMaterial)
{
	UChildActorComponent* CosmeticComponent = nullptr;
	
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		USceneComponent* ComponentToAttachTo = Character->GetMesh();
		CosmeticComponent = NewObject<UChildActorComponent>(Character);
		// 메시 컴포넌트를 부모 컴포넌트로 설정
		CosmeticComponent->SetupAttachment(ComponentToAttachTo);
		// 자식 액터 컴포넌트의 자식 액터 설정
		CosmeticComponent->SetChildActorClass(CosmeticSlotClass);
		// 월드에 등록https://github.com/Rookiss/Inflearn_DX12/commits/main/
		CosmeticComponent->RegisterComponent();

		// 자식 액터가 APanArmorBase기반일 때
		if (APanArmorBase* SpawnedActor = Cast<APanArmorBase>(CosmeticComponent->GetChildActor()))
		{
			if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
			{
				// 메시 컴포넌트가 먼저 Tick되도록 의존성 설정
				SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
			}
			
			// 자식 액터의 프로퍼티 채우기
			SpawnedActor->InitializeActor(InDefaultMesh, InSecondaryMesh, InSkinMaterialSlotName, InSkinMaterial);
		}
	}
	
	// 자식액터가 설정된 자식액터 컴포넌트 반환
	return CosmeticComponent;
}
