#pragma once

#include "PanDefine.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "PanInventoryManagerComponent.generated.h"

class UPanItemTemplate;
class UPanItemInstance;
struct FPanItemRarityProbability;
class UPanEquipmentManagerComponent;
class UPanInventoryManagerComponent;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnInventoryEntryChanged, const FIntPoint&/*ItemSlotPos*/, UPanItemInstance*, int32/*ItemCount*/);

USTRUCT(BlueprintType)
struct FPanInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	UPanItemInstance* Init(int32 InItemTemplateID, int32 InItemCount, EItemRarity InItemRarity);
	void Init(UPanItemInstance* InItemInstance, int32 InItemCount);
	UPanItemInstance* Reset();
	
public:
	UPanItemInstance* GetItemInstance() const { return ItemInstance; }
	int32 GetItemCount() const { return ItemCount; }
	
private:
	friend struct FPanInventoryList;
	friend class UPanInventoryManagerComponent;
	friend class UPanItemManagerComponent;
	
	UPROPERTY()
	TObjectPtr<UPanItemInstance> ItemInstance;

	UPROPERTY()
	int32 ItemCount = 0;
};

USTRUCT(BlueprintType)
struct FPanInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FPanInventoryList() : InventoryManager(nullptr) { }
	FPanInventoryList(UPanInventoryManagerComponent* InOwnerComponent) : InventoryManager(InOwnerComponent) { }

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

private:
	void BroadcastChangedMessage(const FIntPoint& ItemSlotPos, UPanItemInstance* ItemInstance, int32 ItemCount);
	
public:
	const TArray<FPanInventoryEntry>& GetAllEntries() const { return Entries; }
	
private:
	friend class UPanInventoryManagerComponent;
	friend class UPanEquipmentManagerComponent;
	friend class UPanItemManagerComponent;
	
	UPROPERTY()
	TArray<FPanInventoryEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UPanInventoryManagerComponent> InventoryManager;
};

template<>
struct TStructOpsTypeTraits<FPanInventoryList> : public TStructOpsTypeTraitsBase2<FPanInventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

// 인벤토리 창 관리
UCLASS(BlueprintType, Blueprintable)
class UPanInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPanInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	
public:
	// 다른 인벤토리 위치로 아이템이 넘어갈 때
	int32 CanMoveOrMergeItem(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, const FIntPoint& ToItemSlotPos) const;
	int32 CanMoveOrMergeItem(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, const FIntPoint& ToItemSlotPos) const;

	int32 CanMoveOrMergeItem_Quick(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const;
	int32 CanMoveOrMergeItem_Quick(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const;
	
	int32 CanAddItem(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const;
	bool CanRemoveItem(int32 ItemTemplateID, int32 ItemCount, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 TryAddItemByRarity(TSubclassOf<UPanItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 TryAddItemByProbability(TSubclassOf<UPanItemTemplate> ItemTemplateClass, int32 ItemCount, const TArray<FPanItemRarityProbability>& ItemProbabilities);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryRemoveItem(int32 ItemTemplateID, int32 ItemCount);

private:
	void AddItem_Unsafe(const FIntPoint& ItemSlotPos, UPanItemInstance* ItemInstance, int32 ItemCount);
	// 아이템 삭제
	UPanItemInstance* RemoveItem_Unsafe(const FIntPoint& ItemSlotPos, int32 ItemCount);
	
private:
	// 아이템 슬롯에 마크
	void MarkSlotChecks(TArray<bool>& InSlotChecks, bool bIsUsing, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const;
	void MarkSlotChecks(bool bIsUsing, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount);

public:
	bool IsEmpty(const TArray<bool>& InSlotChecks, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const;
	bool IsEmpty(const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const;
	bool IsAllEmpty();
	
	UPanItemInstance* GetItemInstance(const FIntPoint& ItemSlotPos) const;
	int32 GetItemCount(const FIntPoint& ItemSlotPos) const;
	
	const TArray<FPanInventoryEntry>& GetAllEntries() const;
	int32 GetTotalCountByID(int32 ItemTemplateID) const;
	FIntPoint GetInventorySlotCount() const { return InventorySlotCount; }
	TArray<bool>& GetSlotChecks() { return SlotChecks; }

public:
	FOnInventoryEntryChanged OnInventoryEntryChanged;
	
private:
	friend class UPanEquipmentManagerComponent;
	friend class UPanItemManagerComponent;
	
	UPROPERTY(Replicated)
	FPanInventoryList InventoryList;
	
	// 아이템을 아이템창에 배치하기 위한 배열 (테트리스 방식 배치)
	UPROPERTY(Replicated)
	TArray<bool> SlotChecks;
	
	FIntPoint InventorySlotCount = FIntPoint(10, 5);
};
