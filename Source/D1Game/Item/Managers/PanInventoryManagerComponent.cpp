#include "PanInventoryManagerComponent.h"

#include "PanEquipmentManagerComponent.h"
#include "Data/PanItemData.h"
#include "Engine/ActorChannel.h"
#include "Item/PanItemInstance.h"
#include "Item/PanItemTemplate.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanInventoryManagerComponent)

UPanItemInstance* FPanInventoryEntry::Init(int32 InItemTemplateID, int32 InItemCount, EItemRarity InItemRarity)
{
	check(InItemTemplateID > 0 && InItemCount > 0 && InItemRarity != EItemRarity::Count);
	
	UPanItemInstance* NewItemInstance = NewObject<UPanItemInstance>();
	NewItemInstance->Init(InItemTemplateID, InItemRarity);
	Init(NewItemInstance, InItemCount);
	
	return NewItemInstance;
}

void FPanInventoryEntry::Init(UPanItemInstance* InItemInstance, int32 InItemCount)
{
	check(InItemInstance && InItemCount > 0);
	
	ItemInstance = InItemInstance;
	
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
	ItemCount = FMath::Clamp(InItemCount, 1, ItemTemplate.MaxStackCount);
}

UPanItemInstance* FPanInventoryEntry::Reset()
{
	UPanItemInstance* RemovedItemInstance = ItemInstance;
	ItemInstance = nullptr;
	ItemCount = 0;
	
	return RemovedItemInstance;
}

bool FPanInventoryList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FPanInventoryEntry, FPanInventoryList>(Entries, DeltaParams, *this);
}

void FPanInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();

	for (int32 AddedIndex : AddedIndices)
	{
		FPanInventoryEntry& Entry = Entries[AddedIndex];
		if (Entry.ItemInstance)
		{
			const FIntPoint ItemSlotPos = FIntPoint(AddedIndex % InventorySlotCount.X, AddedIndex / InventorySlotCount.X);
			BroadcastChangedMessage(ItemSlotPos, Entry.ItemInstance, Entry.ItemCount);
		}
	}
}

void FPanInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	TArray<int32> AddedIndices;
	AddedIndices.Reserve(FinalSize);

	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();
	
	for (int32 ChangedIndex : ChangedIndices)
	{
		FPanInventoryEntry& Entry = Entries[ChangedIndex];
		if (Entry.ItemInstance)
		{
			AddedIndices.Add(ChangedIndex);
		}
		else
		{
			const FIntPoint ItemSlotPos = FIntPoint(ChangedIndex % InventorySlotCount.X, ChangedIndex / InventorySlotCount.X);
			BroadcastChangedMessage(ItemSlotPos, nullptr, 0);
		}
	}

	for (int32 AddedIndex : AddedIndices)
	{
		FPanInventoryEntry& Entry = Entries[AddedIndex];
		const FIntPoint ItemSlotPos = FIntPoint(AddedIndex % InventorySlotCount.X, AddedIndex / InventorySlotCount.X);
		BroadcastChangedMessage(ItemSlotPos, Entry.ItemInstance, Entry.ItemCount);
	}
}

void FPanInventoryList::BroadcastChangedMessage(const FIntPoint& ItemSlotPos, UPanItemInstance* ItemInstance, int32 ItemCount)
{
	if (InventoryManager->OnInventoryEntryChanged.IsBound())
	{
		InventoryManager->OnInventoryEntryChanged.Broadcast(ItemSlotPos, ItemInstance, ItemCount);
	}
}

UPanInventoryManagerComponent::UPanInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UPanInventoryManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<FPanInventoryEntry>& Entries = InventoryList.Entries;
		Entries.SetNum(InventorySlotCount.X * InventorySlotCount.Y);
	
		for (FPanInventoryEntry& Entry : Entries)
		{
			InventoryList.MarkItemDirty(Entry);
		}

		SlotChecks.SetNumZeroed(InventorySlotCount.X * InventorySlotCount.Y);
	}
}

void UPanInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
	DOREPLIFETIME(ThisClass, SlotChecks);
}

bool UPanInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (FPanInventoryEntry& Entry : InventoryList.Entries)
	{
		UPanItemInstance* ItemInstance = Entry.ItemInstance;
		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}
	
	return bWroteSomething;
}

void UPanInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FPanInventoryEntry& Entry : InventoryList.Entries)
		{
			UPanItemInstance* ItemInstance = Entry.GetItemInstance();
			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

int32 UPanInventoryManagerComponent::CanMoveOrMergeItem(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, const FIntPoint& ToItemSlotPos) const
{
	if (OtherComponent == nullptr)
		return 0;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return 0;
	
	if (ToItemSlotPos.X < 0 || ToItemSlotPos.Y < 0 || ToItemSlotPos.X >= InventorySlotCount.X || ToItemSlotPos.Y >= InventorySlotCount.Y)
		return 0;
	
	const UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromItemSlotPos);
	
	if (this == OtherComponent && FromItemSlotPos == ToItemSlotPos)
		return FromItemCount;
	
	if (FromItemInstance == nullptr || FromItemCount <= 0)
		return 0;
	
	const UPanItemInstance* ToItemInstance = GetItemInstance(ToItemSlotPos);
	const int32 ToItemCount = GetItemCount(ToItemSlotPos);
	
	const int32 FromTemplateID = FromItemInstance->GetItemTemplateID();
	const UPanItemTemplate& FromItemTemplate = UPanItemData::Get().FindItemTemplateByID(FromTemplateID);
	
	if (ToItemInstance)
	{
		const int32 ToTemplateID = ToItemInstance->GetItemTemplateID();
		if (FromTemplateID != ToTemplateID)
			return 0;

		if (FromItemInstance->GetItemRarity() != ToItemInstance->GetItemRarity())
			return 0;
		
		if (FromItemTemplate.MaxStackCount < 2)
			return 0;

		return FMath::Min(FromItemCount + ToItemCount, FromItemTemplate.MaxStackCount) - ToItemCount;
	}
	else
	{
		const FIntPoint& FromItemSlotCount = FromItemTemplate.SlotCount;
		if (ToItemSlotPos.X + FromItemSlotCount.X > InventorySlotCount.X || ToItemSlotPos.Y + FromItemSlotCount.Y > InventorySlotCount.Y)
			return 0;

		if (this == OtherComponent)
		{
			TArray<bool> TempSlotChecks = SlotChecks;
			MarkSlotChecks(TempSlotChecks, false, FromItemSlotPos, FromItemSlotCount);
			
			return IsEmpty(TempSlotChecks, ToItemSlotPos, FromItemSlotCount) ? FromItemCount : 0;
		}
		else
		{
			return IsEmpty(ToItemSlotPos, FromItemSlotCount) ? FromItemCount : 0;
		}
	}
}

int32 UPanInventoryManagerComponent::CanMoveOrMergeItem(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, const FIntPoint& ToItemSlotPos) const
{
	if (OtherComponent == nullptr)
		return 0;

	if (FromEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || FromEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;
	
	if (ToItemSlotPos.X < 0 || ToItemSlotPos.Y < 0 || ToItemSlotPos.X >= InventorySlotCount.X || ToItemSlotPos.Y >= InventorySlotCount.Y)
		return 0;
	
	const UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromEquipmentSlotType);

	if (FromItemInstance == nullptr || FromItemCount <= 0)
		return 0;
	
	const UPanItemInstance* ToItemInstance = GetItemInstance(ToItemSlotPos);
	const int32 ToItemCount = GetItemCount(ToItemSlotPos);
	
	const int32 FromTemplateID = FromItemInstance->GetItemTemplateID();
	const UPanItemTemplate& FromItemTemplate = UPanItemData::Get().FindItemTemplateByID(FromTemplateID);
	
	if (ToItemInstance)
	{
		const int32 ToTemplateID = ToItemInstance->GetItemTemplateID();
		if (FromTemplateID != ToTemplateID)
			return 0;

		if (FromItemInstance->GetItemRarity() != ToItemInstance->GetItemRarity())
			return 0;
		
		if (FromItemTemplate.MaxStackCount < 2)
			return 0;

		return FMath::Min(FromItemCount + ToItemCount, FromItemTemplate.MaxStackCount) - ToItemCount;
	}
	else
	{
		const FIntPoint& FromItemSlotCount = FromItemTemplate.SlotCount;
		if (ToItemSlotPos.X + FromItemSlotCount.X > InventorySlotCount.X || ToItemSlotPos.Y + FromItemSlotCount.Y > InventorySlotCount.Y)
			return 0;

		return IsEmpty(ToItemSlotPos, FromItemSlotCount) ? FromItemCount : 0;
	}
}

int32 UPanInventoryManagerComponent::CanMoveOrMergeItem_Quick(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const
{
	OutToItemSlotPoses.Reset();
	OutToItemCounts.Reset();
	
	if (OtherComponent == nullptr || this == OtherComponent)
		return 0;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return 0;
	
	const UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromItemSlotPos);

	if (FromItemInstance == nullptr)
		return 0;
	
	return CanAddItem(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, OutToItemSlotPoses, OutToItemCounts);
}

int32 UPanInventoryManagerComponent::CanMoveOrMergeItem_Quick(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const
{
	OutToItemSlotPoses.Reset();
	OutToItemCounts.Reset();
	
	if (OtherComponent == nullptr)
		return 0;

	if (FromEquipmentSlotType == EEquipmentSlotType::Unarmed_LeftHand || FromEquipmentSlotType == EEquipmentSlotType::Unarmed_RightHand || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;
	
	const UPanItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromEquipmentSlotType);

	if (FromItemInstance == nullptr)
		return 0;

	return CanAddItem(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, OutToItemSlotPoses, OutToItemCounts);
}

int32 UPanInventoryManagerComponent::CanAddItem(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const
{
	OutToItemSlotPoses.Reset();
	OutToItemCounts.Reset();
	
	if (ItemTemplateID <= 0 || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return 0;
	
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemTemplateID);
	int32 LeftItemCount = ItemCount;
	
	if (ItemTemplate.MaxStackCount > 1)
	{
		const TArray<FPanInventoryEntry>& ToEntries = GetAllEntries();
		
		for (int32 i = 0; i < ToEntries.Num(); i++)
		{
			const FPanInventoryEntry& ToEntry = ToEntries[i];
			const UPanItemInstance* ToItemInstance = ToEntry.GetItemInstance();
			const int32 ToItemCount = ToEntry.GetItemCount();
			
			if (ToItemInstance == nullptr)
				continue;

			if (ToItemInstance->GetItemTemplateID() != ItemTemplateID)
				continue;

			if (ToItemInstance->GetItemRarity() != ItemRarity)
				continue;
			
			if (int32 AddCount = FMath::Min(ToItemCount + LeftItemCount, ItemTemplate.MaxStackCount) - ToItemCount)
			{
				OutToItemSlotPoses.Emplace(i % InventorySlotCount.X, i / InventorySlotCount.X);
				OutToItemCounts.Emplace(AddCount);
				LeftItemCount -= AddCount;

				if (LeftItemCount == 0)
					return ItemCount;
			}
		}
	}
	
	const FIntPoint& ItemSlotCount = ItemTemplate.SlotCount;
	TArray<bool> TempSlotChecks = SlotChecks;
	
	const FIntPoint StartSlotPos = FIntPoint::ZeroValue;
	const FIntPoint EndSlotPos = InventorySlotCount - ItemSlotCount;
	
	for (int32 y = StartSlotPos.Y; y <= EndSlotPos.Y; y++)
	{
		for (int32 x = StartSlotPos.X; x <= EndSlotPos.X; x++)
		{
			int32 Index = y * InventorySlotCount.X + x;
			if (TempSlotChecks.IsValidIndex(Index) == false || TempSlotChecks[Index])
				continue;

			FIntPoint ItemSlotPos = FIntPoint(x, y);
			if (IsEmpty(TempSlotChecks, ItemSlotPos, ItemSlotCount))
			{
				MarkSlotChecks(TempSlotChecks, true, ItemSlotPos, ItemSlotCount);
				
				int32 AddCount = FMath::Min(LeftItemCount, ItemTemplate.MaxStackCount);
				OutToItemSlotPoses.Emplace(ItemSlotPos);
				OutToItemCounts.Emplace(AddCount);
				
				LeftItemCount -= AddCount;
				
				if (LeftItemCount == 0)
					return ItemCount;
			}
		}
	}
	
	return ItemCount - LeftItemCount;
}

bool UPanInventoryManagerComponent::CanRemoveItem(int32 ItemTemplateID, int32 ItemCount, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const
{
	OutToItemSlotPoses.Reset();
	OutToItemCounts.Reset();

	if (ItemTemplateID <= 0 || ItemCount <= 0)
		return false;
	
	TArray<FIntPoint> ToItemSlotPoses;
	TArray<int32> ToItemCounts;

	const TArray<FPanInventoryEntry>& Entries = GetAllEntries();
	
	for (int32 i = Entries.Num() - 1; i >= 0; i--)
	{
		const FPanInventoryEntry& ToEntry = Entries[i];
		if (ToEntry.ItemInstance == nullptr)
			continue;

		if (ToEntry.ItemInstance->GetItemTemplateID() != ItemTemplateID)
			continue;
		
		if (ToEntry.ItemCount < ItemCount)
		{
			ToItemSlotPoses.Emplace(i % InventorySlotCount.X, i / InventorySlotCount.X);
			ToItemCounts.Emplace(ToEntry.ItemCount);
			
			ItemCount -= ToEntry.ItemCount;
		}
		else
		{
			ToItemSlotPoses.Emplace(i % InventorySlotCount.X, i / InventorySlotCount.X);
			ToItemCounts.Emplace(ItemCount);

			OutToItemSlotPoses = ToItemSlotPoses;
			OutToItemCounts = ToItemCounts;
			return true;
		}
	}
	
	return false;
}

int32 UPanInventoryManagerComponent::TryAddItemByRarity(TSubclassOf<UPanItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount)
{
	check(GetOwner()->HasAuthority());

	if (ItemTemplateClass == nullptr || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return 0;
	
	int32 ItemTemplateID = UPanItemData::Get().FindItemTemplateIDByClass(ItemTemplateClass);
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemTemplateID);
	
	TArray<FIntPoint> ToItemSlotPoses;
	TArray<int32> ToItemCounts;

	int32 AddableItemCount = CanAddItem(ItemTemplateID, ItemRarity, ItemCount, ToItemSlotPoses, ToItemCounts);
	if (AddableItemCount > 0)
	{
		TArray<UPanItemInstance*> AddedItemInstances;
		
		for (int32 i = 0; i < ToItemSlotPoses.Num(); i++)
		{
			const FIntPoint& ToItemSlotPos = ToItemSlotPoses[i];
			const int32 ToItemCount = ToItemCounts[i];

			const int32 ToIndex = ToItemSlotPos.Y * InventorySlotCount.X + ToItemSlotPos.X;
			FPanInventoryEntry& ToEntry = InventoryList.Entries[ToIndex];

			if (ToEntry.ItemInstance)
			{
				ToEntry.ItemCount += ToItemCount;
				InventoryList.MarkItemDirty(ToEntry);
			}
			else
			{
				AddedItemInstances.Add(ToEntry.Init(ItemTemplateID, ToItemCount, ItemRarity));
				MarkSlotChecks(true, ToItemSlotPos, ItemTemplate.SlotCount);
				InventoryList.MarkItemDirty(ToEntry);
			}
		}

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			for (UPanItemInstance* AddedItemInstance : AddedItemInstances)
			{
				if (AddedItemInstance)
				{
					AddReplicatedSubObject(AddedItemInstance);
				}
			}
		}
		return AddableItemCount;
	}

	return 0;
}

int32 UPanInventoryManagerComponent::TryAddItemByProbability(TSubclassOf<UPanItemTemplate> ItemTemplateClass, int32 ItemCount, const TArray<FPanItemRarityProbability>& ItemProbabilities)
{
	check(GetOwner()->HasAuthority());

	return TryAddItemByRarity(ItemTemplateClass, UPanItemInstance::DetermineItemRarity(ItemProbabilities), ItemCount);
}

bool UPanInventoryManagerComponent::TryRemoveItem(int32 ItemTemplateID, int32 ItemCount)
{
	check(GetOwner()->HasAuthority());

	if (ItemTemplateID <= 0 || ItemCount <= 0)
		return false;
	
	const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemTemplateID);

	TArray<FIntPoint> ToItemSlotPoses;
	TArray<int32> ToItemCounts;
	
	if (CanRemoveItem(ItemTemplateID, ItemCount, ToItemSlotPoses, ToItemCounts))
	{
		TArray<UPanItemInstance*> RemovedItemInstances;

		for (int32 i = 0; i < ToItemSlotPoses.Num(); i++)
		{
			const FIntPoint& ToItemSlotPos = ToItemSlotPoses[i];
			const int32 ToItemCount = ToItemCounts[i];

			const int32 ToIndex = ToItemSlotPos.Y * InventorySlotCount.X + ToItemSlotPos.X;
			FPanInventoryEntry& ToEntry = InventoryList.Entries[ToIndex];

			ToEntry.ItemCount -= ToItemCount;
			
			if (ToEntry.ItemCount <= 0)
			{
				MarkSlotChecks(false, ToItemSlotPos, ItemTemplate.SlotCount);
				RemovedItemInstances.Add(ToEntry.Reset());
			}

			InventoryList.MarkItemDirty(ToEntry);
		}

		if (IsUsingRegisteredSubObjectList())
		{
			for (UPanItemInstance* RemovedItemInstance : RemovedItemInstances)
			{
				if (RemovedItemInstance)
				{
					RemoveReplicatedSubObject(RemovedItemInstance);
				}
			}
		}
		return true;
	}

	return false;
}

void UPanInventoryManagerComponent::AddItem_Unsafe(const FIntPoint& ItemSlotPos, UPanItemInstance* ItemInstance, int32 ItemCount)
{
	check(GetOwner()->HasAuthority());
	
	const int32 Index = ItemSlotPos.Y * InventorySlotCount.X + ItemSlotPos.X;
	FPanInventoryEntry& Entry = InventoryList.Entries[Index];
	
	if (Entry.GetItemInstance())
	{
		Entry.ItemCount += ItemCount;
		InventoryList.MarkItemDirty(Entry);
	}
	else
	{
		if (ItemInstance == nullptr)
			return;
		
		const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
		
		Entry.Init(ItemInstance, ItemCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
		{
			AddReplicatedSubObject(ItemInstance);
		}

		MarkSlotChecks(true, ItemSlotPos, ItemTemplate.SlotCount);
		InventoryList.MarkItemDirty(Entry);
	}
}

UPanItemInstance* UPanInventoryManagerComponent::RemoveItem_Unsafe(const FIntPoint& ItemSlotPos, int32 ItemCount)
{
	check(GetOwner()->HasAuthority());
	
	const int32 Index = ItemSlotPos.Y * InventorySlotCount.X + ItemSlotPos.X;
	FPanInventoryEntry& Entry = InventoryList.Entries[Index];
	UPanItemInstance* ItemInstance = Entry.GetItemInstance();
	
	Entry.ItemCount -= ItemCount;
	if (Entry.GetItemCount() <= 0)
	{
		const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
		MarkSlotChecks(false, ItemSlotPos, ItemTemplate.SlotCount);
		
		UPanItemInstance* RemovedItemInstance = Entry.Reset();
		if (IsUsingRegisteredSubObjectList() && RemovedItemInstance)
		{
			// 아이템 인스턴스 삭제
			RemoveReplicatedSubObject(RemovedItemInstance);
		}
	}
	
	// 삭제된게 아닐 때 더티플래그 세팅만 하기
	InventoryList.MarkItemDirty(Entry);
	return ItemInstance;
}

void UPanInventoryManagerComponent::MarkSlotChecks(TArray<bool>& InSlotChecks, bool bIsUsing, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const
{
	if (ItemSlotPos.X < 0 || ItemSlotPos.Y < 0)
		return;

	if (ItemSlotPos.X + ItemSlotCount.X > InventorySlotCount.X || ItemSlotPos.Y + ItemSlotCount.Y > InventorySlotCount.Y)
		return;

	const FIntPoint StartSlotPos = ItemSlotPos;
	const FIntPoint EndSlotPos = ItemSlotPos + ItemSlotCount;

	for (int32 y = StartSlotPos.Y; y < EndSlotPos.Y; y++)
	{
		for (int32 x = StartSlotPos.X; x < EndSlotPos.X; x++)
		{
			int32 Index = y * InventorySlotCount.X + x;
			if (InSlotChecks.IsValidIndex(Index))
			{
				// 슬롯에다 마킹
				InSlotChecks[Index] = bIsUsing;
			}
		}
	}
}

void UPanInventoryManagerComponent::MarkSlotChecks(bool bIsUsing, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount)
{
	MarkSlotChecks(SlotChecks, bIsUsing, ItemSlotPos, ItemSlotCount);
}

bool UPanInventoryManagerComponent::IsEmpty(const TArray<bool>& InSlotChecks, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const
{
	if (ItemSlotPos.X < 0 || ItemSlotPos.Y < 0)
		return false;

	if (ItemSlotPos.X + ItemSlotCount.X > InventorySlotCount.X || ItemSlotPos.Y + ItemSlotCount.Y > InventorySlotCount.Y)
		return false;

	const FIntPoint StartSlotPos = ItemSlotPos;
	const FIntPoint EndSlotPos = ItemSlotPos + ItemSlotCount;

	for (int32 y = StartSlotPos.Y; y < EndSlotPos.Y; y++)
	{
		for (int32 x = StartSlotPos.X; x < EndSlotPos.X; x++)
		{
			int32 Index = y * InventorySlotCount.X + x;
			// 비어있는 지 여부 체크
			if (InSlotChecks.IsValidIndex(Index) == false || InSlotChecks[Index])
				return false;
		}
	}
	return true;
}

bool UPanInventoryManagerComponent::IsEmpty(const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const
{
	return IsEmpty(SlotChecks, ItemSlotPos, ItemSlotCount);
}

bool UPanInventoryManagerComponent::IsAllEmpty()
{
	for (FPanInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.GetItemInstance())
			return false;
	}
	return true;
}

UPanItemInstance* UPanInventoryManagerComponent::GetItemInstance(const FIntPoint& ItemSlotPos) const
{
	if (ItemSlotPos.X < 0 || ItemSlotPos.Y < 0 || ItemSlotPos.X >= InventorySlotCount.X || ItemSlotPos.Y >= InventorySlotCount.Y)
		return nullptr;
	
	const TArray<FPanInventoryEntry>& Entries = InventoryList.GetAllEntries();
	const int32 EntryIndex = ItemSlotPos.Y * InventorySlotCount.X + ItemSlotPos.X;
	const FPanInventoryEntry& Entry = Entries[EntryIndex];
	
	return Entry.GetItemInstance();
}

int32 UPanInventoryManagerComponent::GetItemCount(const FIntPoint& ItemSlotPos) const
{
	if (ItemSlotPos.X < 0 || ItemSlotPos.Y < 0 || ItemSlotPos.X >= InventorySlotCount.X || ItemSlotPos.Y >= InventorySlotCount.Y)
		return 0;
	
	const TArray<FPanInventoryEntry>& Entries = InventoryList.GetAllEntries();
	const int32 EntryIndex = ItemSlotPos.Y * InventorySlotCount.X + ItemSlotPos.X;
	const FPanInventoryEntry& Entry = Entries[EntryIndex];
	
	return Entry.GetItemCount();
}

const TArray<FPanInventoryEntry>& UPanInventoryManagerComponent::GetAllEntries() const
{
	return InventoryList.GetAllEntries();
}

int32 UPanInventoryManagerComponent::GetTotalCountByID(int32 ItemTemplateID) const
{
	int32 TotalCount = 0;
	
	for (const FPanInventoryEntry& Entry : GetAllEntries())
	{
		if (UPanItemInstance* ItemInstance = Entry.ItemInstance)
		{
			if (ItemInstance->GetItemTemplateID() == ItemTemplateID)
			{
				TotalCount += Entry.ItemCount;
			}
		}
	}
	
	return TotalCount;
}
