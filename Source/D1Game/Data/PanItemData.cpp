#include "PanItemData.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

#include "Item/PanItemTemplate.h"
#include "Item/Fragments/PanItemFragment_Equipable_Armor.h"
#include "Item/Fragments/PanItemFragment_Equipable_Weapon.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemData)

// 애셋매니저에서 가져오기
const UPanItemData& UPanItemData::Get()
{
	return ULyraAssetManager::Get().GetItemData();
}

#if WITH_EDITORONLY_DATA
// 자동 저장 전 프로퍼티 초기화
void UPanItemData::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	ItemTemplateIDToClass.KeySort([](const int32 A, const int32 B)
	{
		return A < B;
	});

	ItemTemplateClassToID.Empty();
	WeaponItemTemplateClasses.Empty();
	ArmorItemTemplateClasses.Empty();
	
	// 분류된 ItemTemplateIDToClass 기반
	for (const auto& Pair : ItemTemplateIDToClass)
	{
		// ItemTemplateClassToID 채우기
		ItemTemplateClassToID.Emplace(Pair.Value, Pair.Key);

		const UPanItemTemplate* ItemTemplate = Pair.Value.GetDefaultObject();
		if (const UPanItemFragment_Equipable_Weapon* WeaponFragment = ItemTemplate->FindFragmentByClass<UPanItemFragment_Equipable_Weapon>())
		{
			if (WeaponFragment->WeaponType != EWeaponType::Unarmed)
			{
				// WeaponItemTemplateClasses 채우기
				WeaponItemTemplateClasses.Add(Pair.Value);
			}
		}
		else if (ItemTemplate->FindFragmentByClass<UPanItemFragment_Equipable_Armor>())
		{
			// ArmorItemTemplateClasses 채우기
			ArmorItemTemplateClasses.Add(Pair.Value);
		}
	}
}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
// 데이터 유효성 체크
EDataValidationResult UPanItemData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	
	TSet<int32> ItemTemplateIDSet;
	TSet<TSubclassOf<UPanItemTemplate>> ItemTemplateClassSet;

	for (const auto& Pair : ItemTemplateIDToClass)
	{
		// ID Check
		const int32 ItemTemplateID = Pair.Key;
		
		if (ItemTemplateID <= 0)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Invalid ID : [ID : %d]\n"), ItemTemplateID)));
			Result = EDataValidationResult::Invalid;
		}
		
		// 중복체크
		if (ItemTemplateIDSet.Contains(ItemTemplateID))
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Duplicated ID : [ID : %d]\n"), ItemTemplateID)));
			Result = EDataValidationResult::Invalid;
		}
		
		ItemTemplateIDSet.Add(ItemTemplateID);

		// Class Check
		const TSubclassOf<UPanItemTemplate> ItemTemplateClass = Pair.Value;
		
		if (ItemTemplateClass == nullptr)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Invalid Class : [ID : %d]\n"), ItemTemplateID)));
			Result = EDataValidationResult::Invalid;
		}

		if (ItemTemplateClassSet.Contains(ItemTemplateClass))
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Duplicated Class : [ID : %d]\n"), ItemTemplateID)));
			Result = EDataValidationResult::Invalid;
		}
		
		ItemTemplateClassSet.Add(ItemTemplateClass);
	}
	return Result;
}
#endif // WITH_EDITOR

// 아이템 아이디로 클래스 찾기
const UPanItemTemplate& UPanItemData::FindItemTemplateByID(int32 ItemTemplateID) const
{
	const TSubclassOf<UPanItemTemplate>* ItemTemplateClass = ItemTemplateIDToClass.Find(ItemTemplateID);
	ensureAlwaysMsgf(ItemTemplateClass, TEXT("Can't find ItemTemplateClass from ID [%d]"), ItemTemplateID);
	return *(ItemTemplateClass->GetDefaultObject());
}

// 아이템 클래스로 아이디 찾기
int32 UPanItemData::FindItemTemplateIDByClass(TSubclassOf<UPanItemTemplate> ItemTemplateClass) const
{
	const int32* ItemTemplateID = ItemTemplateClassToID.Find(ItemTemplateClass);
	ensureAlwaysMsgf(ItemTemplateID, TEXT("Can't find ItemTemplateID from Class"));
	return *ItemTemplateID;
}

// 모든 아이템 템플릿 클래스 얻기
void UPanItemData::GetAllItemTemplateClasses(TArray<TSubclassOf<UPanItemTemplate>>& OutItemTemplateClasses) const
{
	OutItemTemplateClasses.Reset();
	OutItemTemplateClasses.Reserve(ItemTemplateIDToClass.Num());
	
	for (auto& Pair : ItemTemplateIDToClass)
	{
		OutItemTemplateClasses.Add(Pair.Value);
	}
}
