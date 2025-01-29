#include "PanEquipmentManagerComponent.h"

#include "PanEquipManagerComponent.h"
#include "PanInventoryManagerComponent.h"
#include "PanItemManagerComponent.h"
#include "Character/LyraCharacter.h"
#include "Data/PanItemData.h"
#include "Engine/ActorChannel.h"
#include "Item/PanItemInstance.h"
#include "Item/Fragments/PanItemFragment_Equipable_Armor.h"
#include "Item/Fragments/PanItemFragment_Equipable_Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanEquipmentManagerComponent)

// 초기화
void FPanEquipmentEntry::Init(UPanItemInstance* InItemInstance, int32 InItemCount)
{
	check(InItemInstance && InItemCount > 0);

	UPanEquipManagerComponent* EquipManager = EquipmentManager->GetEquipManager();
	if (EquipManager == nullptr)
		return;

	const UPanItemFragment_Equipable* EquippableFragment = InItemInstance->FindFragmentByClass<UPanItemFragment_Equipable>();
	if (EquippableFragment == nullptr)
		return;
	
	if (ItemInstance)
	{
		EquipManager->Unequip(EquipmentSlotType);
	}
	
	ItemInstance = InItemInstance;
	
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
	ItemCount = FMath::Clamp(InItemCount, 1, ItemTemplate.MaxStackCount);
	
	if (EquippableFragment->EquipmentType == EEquipmentType::Armor || EquipmentManager->IsSameEquipState(EquipmentSlotType, EquipManager->GetCurrentEquipState()))
	{
		EquipManager->Equip(EquipmentSlotType, ItemInstance);
	}
}

// 프로퍼티에 값 비우기
UPanItemInstance* FPanEquipmentEntry::Reset()
{
	UPanEquipManagerComponent* EquipManager = EquipmentManager->GetEquipManager();
	if (EquipManager == nullptr)
		return nullptr;

	if (ItemInstance)
	{
		EquipManager->Unequip(EquipmentSlotType);
	}
	
	UPanItemInstance* RemovedItemInstance = ItemInstance;
	ItemInstance = nullptr;
	ItemCount = 0;
	
	if (EquipmentManager->IsAllEmpty(EquipManager->GetCurrentEquipState()))
	{
		EquipManager->ChangeEquipState(EEquipState::Unarmed);
	}
	
	return RemovedItemInstance;
}

// 변경사항만 네트워크로 보내기 위한 직렬화
bool FPanEquipmentList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FPanEquipmentEntry, FPanEquipmentList>(Entries, DeltaParams,*this);
}

// 동기화로 추가되었을 때
void FPanEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 AddedIndex : AddedIndices)
	{
		FPanEquipmentEntry& Entry = Entries[AddedIndex];
		Entry.EquipmentManager = EquipmentManager;
		Entry.EquipmentSlotType = (EEquipmentSlotType)AddedIndex;
		
		if (Entry.GetItemInstance())
		{
			// 메시지 보내기
			BroadcastChangedMessage((EEquipmentSlotType)AddedIndex, Entry.GetItemInstance(), Entry.GetItemCount());
		}
	}
}

// 동기화로 변경되었을 때
void FPanEquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 ChangedIndex : ChangedIndices)
	{
		const FPanEquipmentEntry& Entry = Entries[ChangedIndex];
		// 메시지 보내기
		BroadcastChangedMessage((EEquipmentSlotType)ChangedIndex, Entry.GetItemInstance(), Entry.GetItemCount());
	}
}

// 변경사항 브로드캐스트
void FPanEquipmentList::BroadcastChangedMessage(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance, int32 ItemCount)
{
	if (EquipmentManager->OnEquipmentEntryChanged.IsBound())
	{
		EquipmentManager->OnEquipmentEntryChanged.Broadcast(EquipmentSlotType, ItemInstance, ItemCount);
	}
}

// 생성자
UPanEquipmentManagerComponent::UPanEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	bWantsInitializeComponent = true;
	// 동기화 허용
    SetIsReplicatedByDefault(true);
}

// 컴포넌트 초기화
void UPanEquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	// 서버에서만
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<FPanEquipmentEntry>& Entries = EquipmentList.Entries;
		// 장비 모음의 여유 공간
		Entries.SetNum((int32)EEquipmentSlotType::Count);

		// 엔트리마다 변경사항 다시 동기화
		for (int32 i = 0; i < Entries.Num(); i++)
		{
			FPanEquipmentEntry& Entry = Entries[i];
			Entry.EquipmentManager = this;
			Entry.EquipmentSlotType = (EEquipmentSlotType)i;
			EquipmentList.MarkItemDirty(Entry);
		}
	}
}

// 동기화할 프로퍼티 등록
void UPanEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}

bool UPanEquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (FPanEquipmentEntry& Entry : EquipmentList.Entries)
	{
		UPanItemInstance* ItemInstance = Entry.ItemInstance;
		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}
	
	return bWroteSomething;
}

void UPanEquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FPanEquipmentEntry& Entry : EquipmentList.Entries)
		{
			UPanItemInstance* ItemInstance = Entry.GetItemInstance();
			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

int32 UPanEquipmentManagerComponent::CanMoveOrMergeEquipment(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (OtherComponent == nullptr)
		return 0;

	if (FromEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || FromEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;

	const UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromEquipmentSlotType);

	if (FromItemInstance == nullptr)
		return 0;

	if (this == OtherComponent && FromEquipmentSlotType == ToEquipmentSlotType)
		return FromItemCount;

	return CanAddEquipment(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, ToEquipmentSlotType);
}

int32 UPanEquipmentManagerComponent::CanMoveOrMergeEquipment(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (OtherComponent == nullptr)
		return 0;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return 0;

	const UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromItemSlotPos);

	if (FromItemInstance == nullptr)
		return 0;

	return CanAddEquipment(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, ToEquipmentSlotType);
}

int32 UPanEquipmentManagerComponent::CanMoveOrMergeEquipment_Quick(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (OtherComponent == nullptr)
		return 0;

	if (FromEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || FromEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;

	if (this == OtherComponent)
		return 0;

	const UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromEquipmentSlotType);

	if (FromItemInstance == nullptr)
		return 0;

	return CanMoveOrMergeEquipment_Quick(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, OutToEquipmentSlotType);
}

int32 UPanEquipmentManagerComponent::CanMoveOrMergeEquipment_Quick(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType) const
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (OtherComponent == nullptr)
		return 0;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return 0;

	const UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromItemSlotPos);

	if (FromItemInstance == nullptr)
		return 0;

	return CanMoveOrMergeEquipment_Quick(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, OutToEquipmentSlotType);
}

int32 UPanEquipmentManagerComponent::CanMoveOrMergeEquipment_Quick(int32 FromItemTemplateID, EItemRarity FromItemRarity, int32 FromItemCount, EEquipmentSlotType& OutToEquipmentSlotType) const
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (FromItemTemplateID <= 0 || FromItemRarity == EItemRarity::Count || FromItemCount <= 0)
		return 0;

	const UPanItemTemplate& FromItemTemplate = UPanItemData::Get().FindItemTemplateByID(FromItemTemplateID);
	const UPanItemFragment_Equipable* FromEquippableFragment = FromItemTemplate.FindFragmentByClass<UPanItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return 0;

	if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UPanItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(FromEquippableFragment);

		for (int32 i = 0; i < (int32)EWeaponSlotType::Count; i++)
		{
			EEquipmentSlotType ToEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType(FromWeaponFragment->WeaponHandType, (EWeaponSlotType)i);
			int32 MovableCount = CanAddEquipment(FromItemTemplateID, FromItemRarity, FromItemCount, ToEquipmentSlotType);
			if (MovableCount > 0)
			{
				OutToEquipmentSlotType = ToEquipmentSlotType;
				return MovableCount;
			}
		}
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Armor)
	{
		const UPanItemFragment_Equipable_Armor* FromArmorFragment = Cast<UPanItemFragment_Equipable_Armor>(FromEquippableFragment);

		EEquipmentSlotType ToEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType(FromArmorFragment->ArmorType);
		int32 MovableCount = CanAddEquipment(FromItemTemplateID, FromItemRarity, FromItemCount, ToEquipmentSlotType);
		if (MovableCount > 0)
		{
			OutToEquipmentSlotType = ToEquipmentSlotType;
			return MovableCount;
		}
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
	{
		EEquipmentSlotType ToEquipmentSlotPick = EEquipmentSlotType::Count;

		for (int32 i = 0; i < (int32)EUtilitySlotType::Count; i++)
		{
			EEquipmentSlotType ToEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType((EUtilitySlotType)i);
			UPanItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);
			if (ToItemInstance == nullptr)
			{
				ToEquipmentSlotPick = ToEquipmentSlotType;
				break;
			}
			else
			{
				const int32 ToItemCount = GetItemCount(ToEquipmentSlotType);
				const UPanItemTemplate& ToItemTemplate = UPanItemData::Get().FindItemTemplateByID(ToItemInstance->GetItemTemplateID());
				if (ToItemTemplate.MaxStackCount > 1 && ToItemCount < ToItemTemplate.MaxStackCount && ToItemInstance->GetItemRarity() == FromItemRarity && ToItemInstance->GetItemTemplateID() == FromItemTemplateID)
				{
					ToEquipmentSlotPick = ToEquipmentSlotType;
					break;
				}
			}
		}

		if (ToEquipmentSlotPick == EEquipmentSlotType::Count)
			return 0;

		int32 MovableCount = CanAddEquipment(FromItemTemplateID, FromItemRarity, FromItemCount, ToEquipmentSlotPick);
		if (MovableCount > 0)
		{
			OutToEquipmentSlotType = ToEquipmentSlotPick;
			return MovableCount;
		}
	}

	return 0;
}

bool UPanEquipmentManagerComponent::CanSwapEquipment(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (OtherComponent == nullptr || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return false;

	const TArray<FPanEquipmentEntry>& FromEntries = OtherComponent->GetAllEntries();
	const FPanEquipmentEntry& FromEntry = FromEntries[(int32)FromEquipmentSlotType];
	UPanItemInstance* FromItemInstance = FromEntry.GetItemInstance();

	if (FromItemInstance == nullptr)
		return false;

	const UPanItemFragment_Equipable* FromEquippableFragment = FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return false;

	if (this == OtherComponent && FromEquipmentSlotType == ToEquipmentSlotType)
		return true;

	const UPanItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);
	if (ToItemInstance == nullptr)
		return false;

	if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UPanItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(FromEquippableFragment);
		if (FromWeaponFragment == nullptr)
			return false;

		EWeaponHandType FromWeaponHandType = FromWeaponFragment->WeaponHandType;
		if (IsSameWeaponHandType(ToEquipmentSlotType, FromWeaponHandType) == false)
			return false;

		return true;
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Armor)
	{
		if (FromEquipmentSlotType != ToEquipmentSlotType)
			return false;

		return true;
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
	{
		if (IsUtilitySlot(ToEquipmentSlotType) == false)
			return false;

		return true;
	}

	return false;
}

bool UPanEquipmentManagerComponent::CanSwapEquipment(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType, FIntPoint& OutToItemSlotPos)
{
	if (OtherComponent == nullptr)
		return false;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return false;

	if (ToEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || ToEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || ToEquipmentSlotType == EEquipmentSlotType::Count)
		return false;

	UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	if (FromItemInstance == nullptr)
		return false;

	const UPanItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);
	if (ToItemInstance == nullptr)
		return false;

	const UPanItemFragment_Equipable* FromEquippableFragment = FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return false;

	if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UPanItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(FromEquippableFragment);
		if (FromWeaponFragment == nullptr)
			return false;

		EWeaponHandType FromWeaponHandType = FromWeaponFragment->WeaponHandType;
		if (IsSameWeaponHandType(ToEquipmentSlotType, FromWeaponHandType) == false)
			return false;
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Armor)
	{
		const UPanItemFragment_Equipable_Armor* FromArmorFragment = Cast<UPanItemFragment_Equipable_Armor>(FromEquippableFragment);
		EEquipmentSlotType FromEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType(FromArmorFragment->ArmorType);

		if (FromEquipmentSlotType != ToEquipmentSlotType)
			return false;
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
	{
		if (IsUtilitySlot(ToEquipmentSlotType) == false)
			return false;
	}

	const UPanItemTemplate& FromItemTemplate = UPanItemData::Get().FindItemTemplateByID(FromItemInstance->GetItemTemplateID());
	const FIntPoint& FromSlotCount = FromItemTemplate.SlotCount;

	const UPanItemTemplate& ToItemTemplate = UPanItemData::Get().FindItemTemplateByID(ToItemInstance->GetItemTemplateID());
	const FIntPoint& ToSlotCount = ToItemTemplate.SlotCount;

	TArray<bool> TempSlotChecks = OtherComponent->SlotChecks;
	OtherComponent->MarkSlotChecks(TempSlotChecks, false, FromItemSlotPos, FromSlotCount);

	if (OtherComponent->IsEmpty(TempSlotChecks, FromItemSlotPos, ToSlotCount))
	{
		OutToItemSlotPos = FromItemSlotPos;
		return true;
	}
	else
	{
		const FIntPoint StartSlotPos = FIntPoint::ZeroValue;
		const FIntPoint EndSlotPos = FromInventorySlotCount - ToSlotCount;

		for (int32 y = StartSlotPos.Y; y <= EndSlotPos.Y; y++)
		{
			for (int32 x = StartSlotPos.X; x <= EndSlotPos.X; x++)
			{
				int32 Index = y * FromInventorySlotCount.X + x;
				if (TempSlotChecks.IsValidIndex(Index) == false || TempSlotChecks[Index])
					continue;

				FIntPoint ToItemSlotPos = FIntPoint(x, y);
				if (OtherComponent->IsEmpty(TempSlotChecks, ToItemSlotPos, ToSlotCount))
				{
					OutToItemSlotPos = ToItemSlotPos;
					return true;
				}
			}
		}
	}

	return false;
}

bool UPanEquipmentManagerComponent::CanSwapEquipment_Quick(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (OtherComponent == nullptr)
		return false;

	if (FromEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || FromEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return false;

	UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	if (FromItemInstance == nullptr)
		return false;

	const UPanItemFragment_Equipable* FromEquippableFragment = FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return false;

	if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UPanItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(FromEquippableFragment);
		if (FromWeaponFragment == nullptr)
			return false;

		if (FindPairItemInstance(FromItemInstance, OutToEquipmentSlotType))
			return true;
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Armor)
	{
		const UPanItemFragment_Equipable_Armor* FromArmorFragment = Cast<UPanItemFragment_Equipable_Armor>(FromEquippableFragment);
		EEquipmentSlotType ToEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType(FromArmorFragment->ArmorType);

		if (GetItemInstance(ToEquipmentSlotType))
		{
			OutToEquipmentSlotType = ToEquipmentSlotType;
			return true;
		}
	}

	return false;
}

bool UPanEquipmentManagerComponent::CanSwapEquipment_Quick(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType, FIntPoint& OutToItemSlotPos)
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (OtherComponent == nullptr)
		return false;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return false;

	UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	if (FromItemInstance == nullptr)
		return false;

	const UPanItemFragment_Equipable* FromEquippableFragment = FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return false;

	const UPanItemInstance* ToItemInstance = nullptr;

	if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UPanItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(FromEquippableFragment);
		if (FromWeaponFragment == nullptr)
			return false;

		ToItemInstance = FindPairItemInstance(FromItemInstance, OutToEquipmentSlotType);
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Armor)
	{
		const UPanItemFragment_Equipable_Armor* FromArmorFragment = Cast<UPanItemFragment_Equipable_Armor>(FromEquippableFragment);
		OutToEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType(FromArmorFragment->ArmorType);

		ToItemInstance = GetItemInstance(OutToEquipmentSlotType);
	}

	if (ToItemInstance == nullptr)
		return false;

	const UPanItemTemplate& FromItemTemplate = UPanItemData::Get().FindItemTemplateByID(FromItemInstance->GetItemTemplateID());
	const FIntPoint& FromSlotCount = FromItemTemplate.SlotCount;

	const UPanItemTemplate& ToItemTemplate = UPanItemData::Get().FindItemTemplateByID(ToItemInstance->GetItemTemplateID());
	const FIntPoint& ToSlotCount = ToItemTemplate.SlotCount;

	TArray<bool> TempSlotChecks = OtherComponent->SlotChecks;
	OtherComponent->MarkSlotChecks(TempSlotChecks, false, FromItemSlotPos, FromSlotCount);

	if (OtherComponent->IsEmpty(TempSlotChecks, FromItemSlotPos, ToSlotCount))
	{
		OutToItemSlotPos = FromItemSlotPos;
		return true;
	}
	else
	{
		const FIntPoint StartSlotPos = FIntPoint::ZeroValue;
		const FIntPoint EndSlotPos = FromInventorySlotCount - ToSlotCount;

		for (int32 y = StartSlotPos.Y; y <= EndSlotPos.Y; y++)
		{
			for (int32 x = StartSlotPos.X; x <= EndSlotPos.X; x++)
			{
				int32 Index = y * FromInventorySlotCount.X + x;
				if (TempSlotChecks.IsValidIndex(Index) == false || TempSlotChecks[Index])
					continue;

				FIntPoint ToItemSlotPos = FIntPoint(x, y);
				if (OtherComponent->IsEmpty(TempSlotChecks, ToItemSlotPos, ToSlotCount))
				{
					OutToItemSlotPos = ToItemSlotPos;
					return true;
				}
			}
		}
	}

	return false;
}

int32 UPanEquipmentManagerComponent::CanAddEquipment(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (ItemTemplateID <= 0 || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return 0;
	
	if (ToEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || ToEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || ToEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;
	
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemTemplateID);
	const UPanItemFragment_Equipable* FromEquippableFragment = ItemTemplate.FindFragmentByClass<UPanItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return 0;
	
	const UPanItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);
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
			const UPanItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(FromEquippableFragment);
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
			const UPanItemFragment_Equipable_Armor* FromArmorFragment = Cast<UPanItemFragment_Equipable_Armor>(FromEquippableFragment);
			return IsSameArmorType(ToEquipmentSlotType, FromArmorFragment->ArmorType) ? ItemCount : 0;
		}
		else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
		{
			return IsUtilitySlot(ToEquipmentSlotType) ? ItemCount : 0;
		}
	}

	return 0;
}

void UPanEquipmentManagerComponent::AddUnarmedEquipments(TSubclassOf<UPanItemTemplate> LeftHandClass, TSubclassOf<UPanItemTemplate> RightHandClass)
{
	check(HasAuthority());

	SetEquipment(EEquipmentSlotType::Unarmed_LeftHand, LeftHandClass, EItemRarity::Poor, 1);
	SetEquipment(EEquipmentSlotType::Unarmed_RightHand, RightHandClass, EItemRarity::Poor, 1);
}

void UPanEquipmentManagerComponent::AddEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance, int32 ItemCount)
{
	check(HasAuthority());

	if (EquipmentSlotType == EEquipmentSlotType::Count || ItemCount <= 0)
		return;
	
	FPanEquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	
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
			// 아이템 추가
			AddReplicatedSubObject(ItemInstance);
		}
		
		EquipmentList.MarkItemDirty(Entry);
	}
}

// 장비 착용
void UPanEquipmentManagerComponent::SetEquipment(EEquipmentSlotType EquipmentSlotType, TSubclassOf<UPanItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount)
{
	check(HasAuthority());

	if (EquipmentSlotType == EEquipmentSlotType::Count || ItemTemplateClass == nullptr || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return;
	
	const int32 ItemTemplateID = UPanItemData::Get().FindItemTemplateIDByClass(ItemTemplateClass);
	// 클래스로 아이디찾고 아이디로 템플릿 찾기
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemTemplateID);

	ItemCount = FMath::Clamp(ItemCount, 1, ItemTemplate.MaxStackCount);
	
	// 템플릿에서 프래그먼트 찾기
	const UPanItemFragment_Equipable* EquippableFragment = ItemTemplate.FindFragmentByClass<UPanItemFragment_Equipable>();
	if (EquippableFragment == nullptr)
		return;

	// 엔트리 비우기
	FPanEquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	Entry.Reset();
	
	// 프래그먼트가 무기일 때
	if (EquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		// 핸드 타입
		const UPanItemFragment_Equipable_Weapon* WeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(EquippableFragment);
		EWeaponHandType WeaponHandType = WeaponFragment->WeaponHandType;

		// 주요 무기 슬롯이라면
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
		// 보조 무기 슬롯이라면
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

	UPanItemInstance* AddedItemInstance = NewObject<UPanItemInstance>();
	AddedItemInstance->Init(ItemTemplateID, ItemRarity);
	// 아이템 인스턴스를 장비 엔트리에 추가
	Entry.Init(AddedItemInstance, ItemCount);

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && AddedItemInstance)
	{
		//AddedItemInstance도 동기화
		AddReplicatedSubObject(AddedItemInstance);
	}
	
	// 변경사항 동기화
	EquipmentList.MarkItemDirty(Entry);

	if (IsWeaponSlot(EquipmentSlotType))
	{
		if (UPanEquipManagerComponent* EquipManager = GetEquipManager())
		{
			EWeaponSlotType WeaponSlotType = UPanEquipManagerComponent::ConvertToWeaponSlotType(EquipmentSlotType);
			EEquipState EquipState = UPanEquipManagerComponent::ConvertToEquipState(WeaponSlotType);
			if (EquipManager->GetCurrentEquipState() != EquipState)
			{
				// 장착 매니저 컴포넌트의 장착 상태 업데이트
				EquipManager->ChangeEquipState(EquipState);
			}
		}
	}
}

UPanItemInstance* UPanEquipmentManagerComponent::RemoveEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, int32 ItemCount)
{
	check(HasAuthority());

	FPanEquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	UPanItemInstance* ItemInstance = Entry.GetItemInstance();

	Entry.ItemCount -= ItemCount;
	if (Entry.GetItemCount() <= 0)
	{
		UPanItemInstance* RemovedItemInstance = Entry.Reset();
		if (IsUsingRegisteredSubObjectList() && RemovedItemInstance)
		{
			RemoveReplicatedSubObject(RemovedItemInstance);
		}
	}

	EquipmentList.MarkItemDirty(Entry);
	return ItemInstance;
}

bool UPanEquipmentManagerComponent::IsWeaponSlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Unarmed_LeftHand <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Secondary_TwoHand);
}

bool UPanEquipmentManagerComponent::IsArmorSlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Helmet <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Foot);
}

bool UPanEquipmentManagerComponent::IsUtilitySlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Utility_Primary <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Utility_Quaternary);
}

bool UPanEquipmentManagerComponent::IsSameEquipState(EEquipmentSlotType EquipmentSlotType, EEquipState WeaponEquipState)
{
	return (((EquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || EquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand) && WeaponEquipState == EEquipState::Unarmed) ||
			((EquipmentSlotType == EEquipmentSlotType::Primary_LeftHand || EquipmentSlotType == EEquipmentSlotType::Primary_RightHand || EquipmentSlotType == EEquipmentSlotType::Primary_TwoHand) && WeaponEquipState == EEquipState::Weapon_Primary) ||
		    ((EquipmentSlotType == EEquipmentSlotType::Secondary_LeftHand || EquipmentSlotType == EEquipmentSlotType::Secondary_RightHand || EquipmentSlotType == EEquipmentSlotType::Secondary_TwoHand) && WeaponEquipState == EEquipState::Weapon_Secondary) ||
		    (EquipmentSlotType == EEquipmentSlotType::Utility_Primary && WeaponEquipState == EEquipState::Utility_Primary) ||
		    (EquipmentSlotType == EEquipmentSlotType::Utility_Secondary && WeaponEquipState == EEquipState::Utility_Secondary) ||
		    (EquipmentSlotType == EEquipmentSlotType::Utility_Tertiary && WeaponEquipState == EEquipState::Utility_Tertiary) ||
		    (EquipmentSlotType == EEquipmentSlotType::Utility_Quaternary && WeaponEquipState == EEquipState::Utility_Quaternary));
}

bool UPanEquipmentManagerComponent::IsSameWeaponHandType(EEquipmentSlotType EquipmentSlotType, EWeaponHandType WeaponHandType)
{
	return (((EquipmentSlotType == EEquipmentSlotType::Primary_LeftHand  || EquipmentSlotType == EEquipmentSlotType::Secondary_LeftHand)  && WeaponHandType == EWeaponHandType::LeftHand)  ||
			((EquipmentSlotType == EEquipmentSlotType::Primary_RightHand || EquipmentSlotType == EEquipmentSlotType::Secondary_RightHand) && WeaponHandType == EWeaponHandType::RightHand) ||
			((EquipmentSlotType == EEquipmentSlotType::Primary_TwoHand   || EquipmentSlotType == EEquipmentSlotType::Secondary_TwoHand)   && WeaponHandType == EWeaponHandType::TwoHand));
}

bool UPanEquipmentManagerComponent::IsSameArmorType(EEquipmentSlotType EquipmentSlotType, EArmorType ArmorType)
{
	return ((EquipmentSlotType == EEquipmentSlotType::Helmet && ArmorType == EArmorType::Helmet) || (EquipmentSlotType == EEquipmentSlotType::Chest && ArmorType == EArmorType::Chest) ||
			(EquipmentSlotType == EEquipmentSlotType::Legs   && ArmorType == EArmorType::Legs)   || (EquipmentSlotType == EEquipmentSlotType::Hands && ArmorType == EArmorType::Hands) ||
			(EquipmentSlotType == EEquipmentSlotType::Foot   && ArmorType == EArmorType::Foot));
}

bool UPanEquipmentManagerComponent::IsPrimaryWeaponSlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Primary_LeftHand <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Primary_TwoHand);
}

bool UPanEquipmentManagerComponent::IsSecondaryWeaponSlot(EEquipmentSlotType EquipmentSlotType)
{
	return (EEquipmentSlotType::Secondary_LeftHand <= EquipmentSlotType && EquipmentSlotType <= EEquipmentSlotType::Secondary_TwoHand);
}

const UPanItemInstance* UPanEquipmentManagerComponent::FindPairItemInstance(const UPanItemInstance* InBaseItemInstance, EEquipmentSlotType& OutEquipmentSlotType) const
{
	if (InBaseItemInstance == nullptr)
		return nullptr;

	const UPanItemFragment_Equipable* BaseEquippableFragment = InBaseItemInstance->FindFragmentByClass<UPanItemFragment_Equipable>();
	if (BaseEquippableFragment == nullptr)
		return nullptr;
	
	UPanItemInstance* SelectedItemInstance = nullptr;

	if (BaseEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UPanItemFragment_Equipable_Weapon* BaseWeaponFragment = Cast<UPanItemFragment_Equipable_Weapon>(BaseEquippableFragment);
		
		const TArray<FPanEquipmentEntry>& Entries = EquipmentList.GetAllEntries();
		for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
		{
			if (IsWeaponSlot((EEquipmentSlotType)i) == false)
				continue;
		
			const FPanEquipmentEntry& Entry = Entries[i];
			if (UPanItemInstance* ItemInstance = Entry.ItemInstance)
			{
				if (const UPanItemFragment_Equipable_Weapon* EquippedWeaponFragment = ItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Weapon>())
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
		const UPanItemFragment_Equipable_Armor* BaseArmorFragment = Cast<UPanItemFragment_Equipable_Armor>(BaseEquippableFragment);
		OutEquipmentSlotType = UPanEquipManagerComponent::ConvertToEquipmentSlotType(BaseArmorFragment->ArmorType);
		SelectedItemInstance = GetItemInstance(OutEquipmentSlotType);
	}

	if (InBaseItemInstance == SelectedItemInstance)
	{
		SelectedItemInstance = nullptr;
	}

	return SelectedItemInstance;
}

bool UPanEquipmentManagerComponent::IsAllEmpty(EEquipState EquipState) const
{
	if (EquipState == EEquipState::Count)
		return true;

	if (EquipState == EEquipState::Unarmed)
		return false;

	bool bAllEmpty = true;
	for (EEquipmentSlotType SlotType : UPanEquipManagerComponent::GetEquipmentSlotsByEquipState(EquipState))
	{
		const FPanEquipmentEntry& Entry = EquipmentList.Entries[(int32)SlotType];
		if (Entry.ItemInstance)
		{
			bAllEmpty = false;
			break;
		}
	}
	return bAllEmpty;
}

ALyraCharacter* UPanEquipmentManagerComponent::GetCharacter() const
{
	return Cast<ALyraCharacter>(GetOwner());
}

ALyraPlayerController* UPanEquipmentManagerComponent::GetPlayerController() const
{
	if (ALyraCharacter* LyraCharacter = GetCharacter())
	{
		return LyraCharacter->GetLyraPlayerController();
	}
	return nullptr;
}

UPanEquipManagerComponent* UPanEquipmentManagerComponent::GetEquipManager() const
{
	UPanEquipManagerComponent* EquipManager = nullptr;
	if (ALyraCharacter* Character = GetCharacter())
	{
		EquipManager = Character->FindComponentByClass<UPanEquipManagerComponent>();
	}
	return EquipManager;
}

UPanItemInstance* UPanEquipmentManagerComponent::GetItemInstance(EEquipmentSlotType EquipmentSlotType) const
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return nullptr;
	
	const TArray<FPanEquipmentEntry>& Entries = EquipmentList.GetAllEntries();
	const FPanEquipmentEntry& Entry = Entries[(int32)EquipmentSlotType];
	
	return Entry.GetItemInstance();
}

int32 UPanEquipmentManagerComponent::GetItemCount(EEquipmentSlotType EquipmentSlotType) const
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return 0;

	const TArray<FPanEquipmentEntry>& Entries = EquipmentList.GetAllEntries();
	const FPanEquipmentEntry& Entry = Entries[(int32)EquipmentSlotType];
	
	return Entry.GetItemCount();
}

const TArray<FPanEquipmentEntry>& UPanEquipmentManagerComponent::GetAllEntries() const
{
	return EquipmentList.GetAllEntries();
}

void UPanEquipmentManagerComponent::GetAllWeaponItemInstances(TArray<UPanItemInstance*>& OutItemInstances) const
{
	OutItemInstances.Reset();
	
	const TArray<FPanEquipmentEntry>& Entries = EquipmentList.GetAllEntries();

	for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
	{
		if (IsWeaponSlot((EEquipmentSlotType)i) == false)
			continue;
		
		const FPanEquipmentEntry& Entry = Entries[i];
		if (Entry.ItemInstance)
		{
			OutItemInstances.Add(Entry.ItemInstance);
		}
	}
}
