#include "PanItemInstance.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "PanItemTemplate.h"
#include "LyraLogChannels.h"
#include "Data/PanItemData.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemInstance)

// 생성자
UPanItemInstance::UPanItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

// 동기화할 프로퍼티
void UPanItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemTemplateID);
	DOREPLIFETIME(ThisClass, ItemRarity);
	DOREPLIFETIME(ThisClass, StatContainer);
}

float UPanItemInstance::GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	return 0;
}

float UPanItemInstance::GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	return 0;
}

// 아이디와 레어도 설정
void UPanItemInstance::Init(int32 InItemTemplateID, EItemRarity InItemRarity)
{
	if (InItemTemplateID <= INDEX_NONE || InItemRarity == EItemRarity::Count)
		return;

	ItemTemplateID = InItemTemplateID;
	ItemRarity = InItemRarity;

	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemTemplateID);
	for (const UPanItemFragment* Fragment : ItemTemplate.Fragments)
	{
		if (Fragment)
		{
			Fragment->OnInstanceCreated(this);
		}
	}
}

#if UE_WITH_IRIS
// 동기화 프래그먼트 등록
void UPanItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

// 스탯 태그 스택 추가
void UPanItemInstance::AddStatTagStack(FGameplayTag StatTag, int32 StackCount)
{
	StatContainer.AddStack(StatTag, StackCount);
}

// 스탯 태그 스택 제거
void UPanItemInstance::RemoveStatTagStack(FGameplayTag StatTag)
{
	StatContainer.RemoveStack(StatTag);
}

// 아이템 레어도 검사
EItemRarity UPanItemInstance::DetermineItemRarity(const TArray<FPanItemRarityProbability>& ItemProbabilities)
{
	float TotalProbability = 0.f;
	for (const FPanItemRarityProbability& ItemProbability : ItemProbabilities)
	{
		TotalProbability += ItemProbability.Probability;
	}

	if (TotalProbability > 100.f)
		return EItemRarity::Count;
	
	float SumProbability = 0.f;
	float RandomValue = FMath::RandRange(0.f, 100.f);

	for (const FPanItemRarityProbability& ItemProbability : ItemProbabilities)
	{
		SumProbability += ItemProbability.Probability;
		if (RandomValue < SumProbability)
		{
			return ItemProbability.Rarity;
		}
	}
	
	return EItemRarity::Count;
}

// 스탯 태그
bool UPanItemInstance::HasStatTag(FGameplayTag StatTag) const
{
	return StatContainer.ContainsTag(StatTag);
}

// 태그를 통해 스택 개수 얻기
int32 UPanItemInstance::GetStackCountByTag(FGameplayTag StatTag) const
{
	return StatContainer.GetStackCount(StatTag);
}

// 클래스의 프래그먼트 찾기
const UPanItemFragment* UPanItemInstance::FindFragmentByClass(TSubclassOf<UPanItemFragment> FragmentClass) const
{
	if (ItemTemplateID > INDEX_NONE && FragmentClass)
	{
		const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemTemplateID);
		return ItemTemplate.FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}
