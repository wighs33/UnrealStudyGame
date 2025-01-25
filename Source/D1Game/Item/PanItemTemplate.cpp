#include "PanItemTemplate.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

#include "Fragments/PanItemFragment_Equipable.h"
#include "Fragments/PanItemFragment_Equipable_Armor.h"
#include "Fragments/PanItemFragment_Equipable_Utility.h"
#include "Fragments/PanItemFragment_Equipable_Weapon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemTemplate)

// 생성자
UPanItemTemplate::UPanItemTemplate(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

#if WITH_EDITOR
// 데이터 유효성 체크
EDataValidationResult UPanItemTemplate::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = UObject::IsDataValid(Context);

	// 슬롯카운트 체크
	if (SlotCount.X < 1 || SlotCount.Y < 1)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("SlotCount is less than 1"))));
		Result = EDataValidationResult::Invalid;
	}
	
	// 최대 스택 개수 체크
	if (MaxStackCount < 1)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("MaxStackCount is less than 1"))));
		Result = EDataValidationResult::Invalid;
	}
	
	const UPanItemFragment_Equipable* FoundEquipable = nullptr;
	for (UPanItemFragment* Fragment : Fragments)
	{
		if (UPanItemFragment_Equipable* CurrentEquippable = Cast<UPanItemFragment_Equipable>(Fragment))
		{
			if (FoundEquipable)
			{
				// 프래그먼트 모음에 UPanItemFragment_Equipable기반이 여러 개일 때 오류 출력
				Context.AddError(FText::FromString(FString::Printf(TEXT("Duplicated Equippable Fragment"))));
				return EDataValidationResult::Invalid;
			}

			FoundEquipable = CurrentEquippable;
		}
	}
	
	if (FoundEquipable)
	{
		// 장비 타입 유효성 체크
		if (FoundEquipable->EquipmentType == EEquipmentType::Count)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Equipment Type is Invalid : [EEquipmentType::Count]"))));
			return EDataValidationResult::Invalid;
		}
		
		// 갑옷일 때
		if (FoundEquipable->EquipmentType == EEquipmentType::Armor)
		{
			const UPanItemFragment_Equipable_Armor* ArmorFragment = Cast<UPanItemFragment_Equipable_Armor>(FoundEquipable);
			
			// 갑옷 타입 유효성 체크
			if (ArmorFragment->ArmorType == EArmorType::Count)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Armor Type is Invalid : [EArmorType::Count]"))));
				Result = EDataValidationResult::Invalid;
			}
		}
		// 무기일 때
		else if (FoundEquipable->EquipmentType == EEquipmentType::Weapon)
		{
			const UPanItemFragment_Equipable_Weapon* WeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(FoundEquipable);
			
			// 무기 타입 유효성 체크
			if (WeaponFragment->WeaponType == EWeaponType::Count)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Weapon Type is Invalid : [EWeaponType::Count]"))));
				Result = EDataValidationResult::Invalid;
			}
			
			// 무기 착용 방식 유효성 체크
			if (WeaponFragment->WeaponHandType == EWeaponHandType::Count)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Weapon Hand Type is Invalid : [EWeaponHandType::Count]"))));
				Result = EDataValidationResult::Invalid;
			}
		}
		// 용도일 때
		else if (FoundEquipable->EquipmentType == EEquipmentType::Utility)
		{
			const UPanItemFragment_Equipable_Utility* UtilityFragment = Cast<UPanItemFragment_Equipable_Utility>(FoundEquipable);

			// 용도 타입 유효성 체크
			if (UtilityFragment->UtilityType == EUtilityType::Count)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Utility Type is Invalid : [EUtilityType::Count]"))));
				Result = EDataValidationResult::Invalid;
			}
		}

		// 갑옷이나 무기일 때
		if (FoundEquipable->EquipmentType == EEquipmentType::Armor || FoundEquipable->EquipmentType == EEquipmentType::Weapon)
		{
			if (MaxStackCount != 1)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Armor or Weapon Type must have MaxStackCount of 1: [MaxStackCount != 1]"))));
				Result = EDataValidationResult::Invalid;
			}
		}
	}
	
	return Result;
}
#endif // WITH_EDITOR

// 프래그먼트 모음에서 클래스를 통해 프래그먼트 찾기
const UPanItemFragment* UPanItemTemplate::FindFragmentByClass(TSubclassOf<UPanItemFragment> FragmentClass) const
{
	if (FragmentClass)
	{
		for (UPanItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}
	return nullptr;
}
