﻿#include "D1EquipmentManagerComponent.h"

//#include "D1EquipManagerComponent.h"
//#include "D1InventoryManagerComponent.h"
//#include "D1ItemManagerComponent.h"
#include "Character/LyraCharacter.h"
#include "Data/D1ItemData.h"
#include "Engine/ActorChannel.h"
#include "Item/D1ItemInstance.h"
#include "Item/Fragments/D1ItemFragment_Equipable_Armor.h"
#include "Item/Fragments/D1ItemFragment_Equipable_Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(D1EquipmentManagerComponent)

void FD1EquipmentEntry::Init(UD1ItemInstance* InItemInstance, int32 InItemCount)
{
	check(InItemInstance && InItemCount > 0);

	/*UD1EquipManagerComponent* EquipManager = EquipmentManager->GetEquipManager();
	if (EquipManager == nullptr)
		return;*/

	const UD1ItemFragment_Equipable* EquippableFragment = InItemInstance->FindFragmentByClass<UD1ItemFragment_Equipable>();
	if (EquippableFragment == nullptr)
		return;
	
	if (ItemInstance)
	{
		//EquipManager->Unequip(EquipmentSlotType);
	}
	
	ItemInstance = InItemInstance;
	
	const UD1ItemTemplate& ItemTemplate = UD1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
	ItemCount = FMath::Clamp(InItemCount, 1, ItemTemplate.MaxStackCount);
	
	//if (EquippableFragment->EquipmentType == EEquipmentType::Armor || EquipmentManager->IsSameEquipState(EquipmentSlotType, EquipManager->GetCurrentEquipState()))
	{
		//EquipManager->Equip(EquipmentSlotType, ItemInstance);
	}
}

UD1ItemInstance* FD1EquipmentEntry::Reset()
{
	/*UD1EquipManagerComponent* EquipManager = EquipmentManager->GetEquipManager();
	if (EquipManager == nullptr)
		return nullptr;*/

	if (ItemInstance)
	{
		//EquipManager->Unequip(EquipmentSlotType);
	}
	
	UD1ItemInstance* RemovedItemInstance = ItemInstance;
	ItemInstance = nullptr;
	ItemCount = 0;
	
	//if (EquipmentManager->IsAllEmpty(EquipManager->GetCurrentEquipState()))
	{
		//EquipManager->ChangeEquipState(EEquipState::Unarmed);
	}
	
	return RemovedItemInstance;
}

bool FD1EquipmentList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FD1EquipmentEntry, FD1EquipmentList>(Entries, DeltaParams,*this);
}

void FD1EquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 AddedIndex : AddedIndices)
	{
		FD1EquipmentEntry& Entry = Entries[AddedIndex];
		Entry.EquipmentManager = EquipmentManager;
		Entry.EquipmentSlotType = (EEquipmentSlotType)AddedIndex;
		
		if (Entry.GetItemInstance())
		{
			BroadcastChangedMessage((EEquipmentSlotType)AddedIndex, Entry.GetItemInstance(), Entry.GetItemCount());
		}
	}
}

void FD1EquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 ChangedIndex : ChangedIndices)
	{
		const FD1EquipmentEntry& Entry = Entries[ChangedIndex];
		BroadcastChangedMessage((EEquipmentSlotType)ChangedIndex, Entry.GetItemInstance(), Entry.GetItemCount());
	}
}

void FD1EquipmentList::BroadcastChangedMessage(EEquipmentSlotType EquipmentSlotType, UD1ItemInstance* ItemInstance, int32 ItemCount)
{
	if (EquipmentManager->OnEquipmentEntryChanged.IsBound())
	{
		EquipmentManager->OnEquipmentEntryChanged.Broadcast(EquipmentSlotType, ItemInstance, ItemCount);
	}
}

UD1EquipmentManagerComponent::UD1EquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	bWantsInitializeComponent = true;
    SetIsReplicatedByDefault(true);
}

void UD1EquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<FD1EquipmentEntry>& Entries = EquipmentList.Entries;
		Entries.SetNum((int32)EEquipmentSlotType::Count);

		for (int32 i = 0; i < Entries.Num(); i++)
		{
			FD1EquipmentEntry& Entry = Entries[i];
			Entry.EquipmentManager = this;
			Entry.EquipmentSlotType = (EEquipmentSlotType)i;
			EquipmentList.MarkItemDirty(Entry);
		}
	}
}

void UD1EquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}

bool UD1EquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (FD1EquipmentEntry& Entry : EquipmentList.Entries)
	{
		UD1ItemInstance* ItemInstance = Entry.ItemInstance;
		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}
	
	return bWroteSomething;
}

void UD1EquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FD1EquipmentEntry& Entry : EquipmentList.Entries)
		{
			UD1ItemInstance* ItemInstance = Entry.GetItemInstance();
			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

int32 UD1EquipmentManagerComponent::CanAddEquipment(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (ItemTemplateID <= 0 || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return 0;
	
	if (ToEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || ToEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || ToEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;
	
	const UD1ItemTemplate& ItemTemplate = UD1ItemData::Get().FindItemTemplateByID(ItemTemplateID);
	const UD1ItemFragment_Equipable* FromEquippableFragment = ItemTemplate.FindFragmentByClass<UD1ItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return 0;
	
	const UD1ItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);
	const int32 ToItemCount = GetItemCount(ToEquipmentSlotType);

	if (ToItemInstance)
	{
		const int32 ToTemplateID = ToItemInstance->GetItemTemplateID();
		if (ItemTemplateID != ToTemplateID)
			return 0;

		if (ItemRarity != ToItemInstance->GetItemRarity())
			return 0;
		
		if (ItemTemplate.MaxStackCount < 2)
			return 0;

		return FMath::Min(ItemCount + ToItemCount, ItemTemplate.MaxStackCount) - ToItemCount;
	}
	else
	{
		if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
		{
			const UD1ItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UD1ItemFragment_Equipable_Weapon>(FromEquippableFragment);
			EWeaponHandType FromWeaponHandType = FromWeaponFragment->WeaponHandType;

			if (IsSameWeaponHandType(ToEquipmentSlotType, FromWeaponHandType) == false)
				return 0;
		
			if (IsPrimaryWeaponSlot(ToEquipmentSlotType))
			{
				if (FromWeaponHandType == EWeaponHandType::LeftHand || FromWeaponHandType == EWeaponHandType::RightHand)
				{
					return (GetItemInstance(EEquipmentSlotType::Primary_TwoHand) == nullptr) ? ItemCount : 0;
				}
				else if (FromWeaponHandType == EWeaponHandType::TwoHand)
				{
					return (GetItemInstance(EEquipmentSlotType::Primary_LeftHand) == nullptr && GetItemInstance(EEquipmentSlotType::Primary_RightHand) == nullptr) ? ItemCount : 0;
				}
			}
			else if (IsSecondaryWeaponSlot(ToEquipmentSlotType))
			{
				if (FromWeaponHandType == EWeaponHandType::LeftHand || FromWeaponHandType == EWeaponHandType::RightHand)
				{
					return (GetItemInstance(EEquipmentSlotType::Secondary_TwoHand) == nullptr) ? ItemCount : 0;
				}
				else if (FromWeaponHandType == EWeaponHandType::TwoHand)
				{
					return (GetItemInstance(EEquipmentSlotType::Secondary_LeftHand) == nullptr && GetItemInstance(EEquipmentSlotType::Secondary_RightHand) == nullptr) ? ItemCount : 0;
				}
			}
		}
		else if (FromEquippableFragment->EquipmentType == EEquipmentType::Armor)
		{
			const UD1ItemFragment_Equipable_Armor* FromArmorFragment = Cast<UD1ItemFragment_Equipable_Armor>(FromEquippableFragment);
			return IsSameArmorType(ToEquipmentSlotType, FromArmorFragment->ArmorType) ? ItemCount : 0;
		}
		else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
		{
			return IsUtilitySlot(ToEquipmentSlotType) ? ItemCount : 0;
		}
	}

	return 0;
}

void UD1EquipmentManagerComponent::AddUnarmedEquipments(TSubclassOf<UD1ItemTemplate> LeftHandClass, TSubclassOf<UD1ItemTemplate> RightHandClass)
{
	check(HasAuthority());

	SetEquipment(EEquipmentSlotType::Unarmed_LeftHand, LeftHandClass, EItemRarity::Poor, 1);
	SetEquipment(EEquipmentSlotType::Unarmed_RightHand, RightHandClass, EItemRarity::Poor, 1);
}

void UD1EquipmentManagerComponent::AddEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, UD1ItemInstance* ItemInstance, int32 ItemCount)
{
	check(HasAuthority());

	if (EquipmentSlotType == EEquipmentSlotType::Count || ItemCount <= 0)
		return;
	
	FD1EquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	
	if (Entry.GetItemInstance())
	{
		Entry.ItemCount += ItemCount;
		EquipmentList.MarkItemDirty(Entry);
	}
	else
	{
		if (ItemInstance == nullptr)
			return;
		
		Entry.Init(ItemInstance, ItemCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
		{
			AddReplicatedSubObject(ItemInstance);
		}
		
		EquipmentList.MarkItemDirty(Entry);
	}
}

void UD1EquipmentManagerComponent::SetEquipment(EEquipmentSlotType EquipmentSlotType, TSubclassOf<UD1ItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount)
{
	check(HasAuthority());

	if (EquipmentSlotType == EEquipmentSlotType::Count || ItemTemplateClass == nullptr || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return;
	
	const int32 ItemTemplateID = UD1ItemData::Get().FindItemTemplateIDByClass(ItemTemplateClass);
	const UD1ItemTemplate& ItemTemplate = UD1ItemData::Get().FindItemTemplateByID(ItemTemplateID);

	ItemCount = FMath::Clamp(ItemCount, 1, ItemTemplate.MaxStackCount);
	
	const UD1ItemFragment_Equipable* EquippableFragment = ItemTemplate.FindFragmentByClass<UD1ItemFragment_Equipable>();
	if (EquippableFragment == nullptr)
		return;

	FD1EquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	Entry.Reset();
	
	if (EquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UD1ItemFragment_Equipable_Weapon* WeaponFragment = Cast<UD1ItemFragment_Equipable_Weapon>(EquippableFragment);
		EWeaponHandType WeaponHandType = WeaponFragment->WeaponHandType;

		if (IsPrimaryWeaponSlot(EquipmentSlotType))
		{
			if (WeaponHandType == EWeaponHandType::LeftHand || WeaponHandType == EWeaponHandType::RightHand)
			{
				RemoveEquipment_Unsafe(EEquipmentSlotType::Primary_TwoHand, 1);
			}
			else if (WeaponHandType == EWeaponHandType::TwoHand)
			{
				RemoveEquipment_Unsafe(EEquipmentSlotType::Primary_LeftHand, 1);
				RemoveEquipment_Unsafe(EEquipmentSlotType::Primary_RightHand, 1);
			}
		}
		else if (IsSecondaryWeaponSlot(EquipmentSlotType))
		{
			if (WeaponHandType == EWeaponHandType::LeftHand || WeaponHandType == EWeaponHandType::RightHand)
			{
				RemoveEquipment_Unsafe(EEquipmentSlotType::Secondary_TwoHand, 1);
			}
			else if (WeaponHandType == EWeaponHandType::TwoHand)
			{
				RemoveEquipment_Unsafe(EEquipmentSlotType::Secondary_LeftHand, 1);
				RemoveEquipment_Unsafe(EEquipmentSlotType::Secondary_RightHand, 1);
			}
		}
	}

	UD1ItemInstance* AddedItemInstance = NewObject<UD1ItemInstance>();
	AddedItemInstance->Init(ItemTemplateID, ItemRarity);
	Entry.Init(AddedItemInstance, ItemCount);

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && AddedItemInstance)
	{
		AddReplicatedSubObject(AddedItemInstance);
	}
	
	EquipmentList.MarkItemDirty(Entry);

	if (IsWeaponSlot(EquipmentSlotType))
	{
		/*if (UD1EquipManagerComponent* EquipManager = GetEquipManager())
		{
			EWeaponSlotType WeaponSlotType = UD1EquipManagerComponent::ConvertToWeaponSlotType(EquipmentSlotType);
			EEquipState EquipState = UD1EquipManagerComponent::ConvertToEquipState(WeaponSlotType);
			if (EquipManager->GetCurrentEquipState() != EquipState)
			{
				EquipManager->ChangeEquipState(EquipState);
			}
		}*/
	}
}

UD1ItemInstance* UD1EquipmentManagerComponent::RemoveEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, int32 ItemCount)
{
	check(HasAuthority());

	FD1EquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	UD1ItemInstance* ItemInstance = Entry.GetItemInstance();

	Entry.ItemCount -= ItemCount;
	if (Entry.GetItemCount() <= 0)
	{
		UD1ItemInstance* RemovedItemInstance = Entry.Reset();
		if (IsUsingRegisteredSubObjectList() && RemovedItemInstance)
		{
			RemoveReplicatedSubObject(RemovedItemInstance);
		}
	}

	EquipmentList.MarkItemDirty(Entry);
	return ItemInstance;
}

bool UD1EquipmentManagerComponent::IsWeaponSlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Unarmed_LeftHand <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Secondary_TwoHand);
}

bool UD1EquipmentManagerComponent::IsArmorSlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Helmet <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Foot);
}

bool UD1EquipmentManagerComponent::IsUtilitySlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Utility_Primary <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Utility_Quaternary);
}

bool UD1EquipmentManagerComponent::IsSameEquipState(EEquipmentSlotType EquipmentSlotType, EEquipState WeaponEquipState)
{
	return (((EquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || EquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand) && WeaponEquipState == EEquipState::Unarmed) ||
			((EquipmentSlotType == EEquipmentSlotType::Primary_LeftHand || EquipmentSlotType == EEquipmentSlotType::Primary_RightHand || EquipmentSlotType == EEquipmentSlotType::Primary_TwoHand) && WeaponEquipState == EEquipState::Weapon_Primary) ||
		    ((EquipmentSlotType == EEquipmentSlotType::Secondary_LeftHand || EquipmentSlotType == EEquipmentSlotType::Secondary_RightHand || EquipmentSlotType == EEquipmentSlotType::Secondary_TwoHand) && WeaponEquipState == EEquipState::Weapon_Secondary) ||
		    (EquipmentSlotType == EEquipmentSlotType::Utility_Primary && WeaponEquipState == EEquipState::Utility_Primary) ||
		    (EquipmentSlotType == EEquipmentSlotType::Utility_Secondary && WeaponEquipState == EEquipState::Utility_Secondary) ||
		    (EquipmentSlotType == EEquipmentSlotType::Utility_Tertiary && WeaponEquipState == EEquipState::Utility_Tertiary) ||
		    (EquipmentSlotType == EEquipmentSlotType::Utility_Quaternary && WeaponEquipState == EEquipState::Utility_Quaternary));
}

bool UD1EquipmentManagerComponent::IsSameWeaponHandType(EEquipmentSlotType EquipmentSlotType, EWeaponHandType WeaponHandType)
{
	return (((EquipmentSlotType == EEquipmentSlotType::Primary_LeftHand  || EquipmentSlotType == EEquipmentSlotType::Secondary_LeftHand)  && WeaponHandType == EWeaponHandType::LeftHand)  ||
			((EquipmentSlotType == EEquipmentSlotType::Primary_RightHand || EquipmentSlotType == EEquipmentSlotType::Secondary_RightHand) && WeaponHandType == EWeaponHandType::RightHand) ||
			((EquipmentSlotType == EEquipmentSlotType::Primary_TwoHand   || EquipmentSlotType == EEquipmentSlotType::Secondary_TwoHand)   && WeaponHandType == EWeaponHandType::TwoHand));
}

bool UD1EquipmentManagerComponent::IsSameArmorType(EEquipmentSlotType EquipmentSlotType, EArmorType ArmorType)
{
	return ((EquipmentSlotType == EEquipmentSlotType::Helmet && ArmorType == EArmorType::Helmet) || (EquipmentSlotType == EEquipmentSlotType::Chest && ArmorType == EArmorType::Chest) ||
			(EquipmentSlotType == EEquipmentSlotType::Legs   && ArmorType == EArmorType::Legs)   || (EquipmentSlotType == EEquipmentSlotType::Hands && ArmorType == EArmorType::Hands) ||
			(EquipmentSlotType == EEquipmentSlotType::Foot   && ArmorType == EArmorType::Foot));
}

bool UD1EquipmentManagerComponent::IsPrimaryWeaponSlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Primary_LeftHand <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Primary_TwoHand);
}

bool UD1EquipmentManagerComponent::IsSecondaryWeaponSlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Secondary_LeftHand <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Secondary_TwoHand);
}

const UD1ItemInstance* UD1EquipmentManagerComponent::FindPairItemInstance(const UD1ItemInstance* InBaseItemInstance, EEquipmentSlotType& OutEquipmentSlotType) const
{
	if (InBaseItemInstance == nullptr)
		return nullptr;

	const UD1ItemFragment_Equipable* BaseEquippableFragment = InBaseItemInstance->FindFragmentByClass<UD1ItemFragment_Equipable>();
	if (BaseEquippableFragment == nullptr)
		return nullptr;
	
	UD1ItemInstance* SelectedItemInstance = nullptr;

	if (BaseEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UD1ItemFragment_Equipable_Weapon* BaseWeaponFragment = Cast<UD1ItemFragment_Equipable_Weapon>(BaseEquippableFragment);
		
		const TArray<FD1EquipmentEntry>& Entries = EquipmentList.GetAllEntries();
		for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
		{
			if (IsWeaponSlot((EEquipmentSlotType)i) == false)
				continue;
		
			const FD1EquipmentEntry& Entry = Entries[i];
			if (UD1ItemInstance* ItemInstance = Entry.ItemInstance)
			{
				if (const UD1ItemFragment_Equipable_Weapon* EquippedWeaponFragment = ItemInstance->FindFragmentByClass<UD1ItemFragment_Equipable_Weapon>())
				{
					if ((BaseWeaponFragment->WeaponType == EquippedWeaponFragment->WeaponType) && (BaseWeaponFragment->WeaponHandType == EquippedWeaponFragment->WeaponHandType))
					{
						if (InBaseItemInstance == ItemInstance)
						{
							SelectedItemInstance = nullptr;
							break;
						}

						if (SelectedItemInstance == nullptr)
						{
							SelectedItemInstance = ItemInstance;
							OutEquipmentSlotType = (EEquipmentSlotType)i;
						}
					}
				}
			}
		}
	}
	else if (BaseEquippableFragment->EquipmentType == EEquipmentType::Armor)
	{
		/*const UD1ItemFragment_Equipable_Armor* BaseArmorFragment = Cast<UD1ItemFragment_Equipable_Armor>(BaseEquippableFragment);
		OutEquipmentSlotType = UD1EquipManagerComponent::ConvertToEquipmentSlotType(BaseArmorFragment->ArmorType);
		SelectedItemInstance = GetItemInstance(OutEquipmentSlotType);*/
	}

	if (InBaseItemInstance == SelectedItemInstance)
	{
		SelectedItemInstance = nullptr;
	}

	return SelectedItemInstance;
}

bool UD1EquipmentManagerComponent::IsAllEmpty(EEquipState EquipState) const
{
	if (EquipState == EEquipState::Count)
		return true;

	if (EquipState == EEquipState::Unarmed)
		return false;

	bool bAllEmpty = true;
	/*for (EEquipmentSlotType SlotType : UD1EquipManagerComponent::GetEquipmentSlotsByEquipState(EquipState))
	{
		const FD1EquipmentEntry& Entry = EquipmentList.Entries[(int32)SlotType];
		if (Entry.ItemInstance)
		{
			bAllEmpty = false;
			break;
		}
	}*/
	return bAllEmpty;
}

ALyraCharacter* UD1EquipmentManagerComponent::GetCharacter() const
{
	return Cast<ALyraCharacter>(GetOwner());
}

ALyraPlayerController* UD1EquipmentManagerComponent::GetPlayerController() const
{
	if (ALyraCharacter* LyraCharacter = GetCharacter())
	{
		return LyraCharacter->GetLyraPlayerController();
	}
	return nullptr;
}

UD1ItemInstance* UD1EquipmentManagerComponent::GetItemInstance(EEquipmentSlotType EquipmentSlotType) const
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return nullptr;
	
	const TArray<FD1EquipmentEntry>& Entries = EquipmentList.GetAllEntries();
	const FD1EquipmentEntry& Entry = Entries[(int32)EquipmentSlotType];
	
	return Entry.GetItemInstance();
}

int32 UD1EquipmentManagerComponent::GetItemCount(EEquipmentSlotType EquipmentSlotType) const
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return 0;

	const TArray<FD1EquipmentEntry>& Entries = EquipmentList.GetAllEntries();
	const FD1EquipmentEntry& Entry = Entries[(int32)EquipmentSlotType];
	
	return Entry.GetItemCount();
}

const TArray<FD1EquipmentEntry>& UD1EquipmentManagerComponent::GetAllEntries() const
{
	return EquipmentList.GetAllEntries();
}

void UD1EquipmentManagerComponent::GetAllWeaponItemInstances(TArray<UD1ItemInstance*>& OutItemInstances) const
{
	OutItemInstances.Reset();
	
	const TArray<FD1EquipmentEntry>& Entries = EquipmentList.GetAllEntries();

	for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
	{
		if (IsWeaponSlot((EEquipmentSlotType)i) == false)
			continue;
		
		const FD1EquipmentEntry& Entry = Entries[i];
		if (Entry.ItemInstance)
		{
			OutItemInstances.Add(Entry.ItemInstance);
		}
	}
}
