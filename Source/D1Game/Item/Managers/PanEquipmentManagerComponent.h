#pragma once

#include "PanDefine.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "PanEquipmentManagerComponent.generated.h"

class UPanEquipManagerComponent;
class ALyraCharacter;
class ALyraPlayerController;
class UPanItemInstance;
class UPanItemTemplate;
class UPanEquipmentManagerComponent;
//class UPanInventoryManagerComponent;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentEntryChanged, EEquipmentSlotType, UPanItemInstance*, int32/*ItemCount*/);

// 장비 엔트리 (싱글턴)
USTRUCT(BlueprintType)
struct FPanEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
private:
	void Init(UPanItemInstance* InItemInstance, int32 InItemCount);
	UPanItemInstance* Reset();
	
public:
	UPanItemInstance* GetItemInstance() const { return ItemInstance; }
	int32 GetItemCount() const { return ItemCount; }
	
private:
	friend struct FPanEquipmentList;
	friend class UPanEquipmentManagerComponent;
	friend class UPanItemManagerComponent;

	// 아이템 인스턴스
	UPROPERTY()
	TObjectPtr<UPanItemInstance> ItemInstance;

	// 아이템 개수
	UPROPERTY()
	int32 ItemCount = 0;

private:
	// 장비 슬롯 타입
	UPROPERTY(NotReplicated)
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;
	
	// 장비 매니저
	UPROPERTY(NotReplicated)
	TObjectPtr<UPanEquipmentManagerComponent> EquipmentManager;
};

// 장비 리스트
USTRUCT(BlueprintType)
struct FPanEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FPanEquipmentList() : EquipmentManager(nullptr) { }
	FPanEquipmentList(UPanEquipmentManagerComponent* InOwnerComponent) : EquipmentManager(InOwnerComponent) { }
	
public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	
private:
	void BroadcastChangedMessage(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance, int32 ItemCount);
	
public:
	const TArray<FPanEquipmentEntry>& GetAllEntries() const { return Entries; }
	
private:
	friend class UPanEquipmentManagerComponent;
	friend class UPanInventoryManagerComponent;
	friend class UPanItemManagerComponent;

	// 장비 엔트리 모음
	UPROPERTY()
	TArray<FPanEquipmentEntry> Entries;
	
	// 장비 매니저
	UPROPERTY(NotReplicated)
	TObjectPtr<UPanEquipmentManagerComponent> EquipmentManager;
};

template<>
struct TStructOpsTypeTraits<FPanEquipmentList> : public TStructOpsTypeTraitsBase2<FPanEquipmentList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

// 장비 매니저 컴포넌트
UCLASS(BlueprintType)
class UPanEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
	UPanEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	
public:
	/*
	int32 CanMoveOrMergeEquipment(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType) const;
	
	int32 CanMoveOrMergeEquipment_Quick(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment_Quick(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment_Quick(int32 FromItemTemplateID, EItemRarity FromItemRarity, int32 FromItemCount, EEquipmentSlotType& OutToEquipmentSlotType) const;

	bool CanSwapEquipment(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const;
	bool CanSwapEquipment(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType, FIntPoint& OutToItemSlotPos);
	
	bool CanSwapEquipment_Quick(UPanEquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const;
	bool CanSwapEquipment_Quick(UPanInventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType, FIntPoint& OutToItemSlotPos);
	*/
	
	int32 CanAddEquipment(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, EEquipmentSlotType ToEquipmentSlotType) const;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddUnarmedEquipments(TSubclassOf<UPanItemTemplate> LeftHandClass, TSubclassOf<UPanItemTemplate> RightHandClass);
	
public:
	void AddEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance, int32 ItemCount);
	UPanItemInstance* RemoveEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, int32 ItemCount);
	void SetEquipment(EEquipmentSlotType EquipmentSlotType, TSubclassOf<UPanItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount);
	
public:
	static bool IsWeaponSlot(EEquipmentSlotType EquipmentSlotType);
	static bool IsArmorSlot(EEquipmentSlotType EquipmentSlotType);
	static bool IsUtilitySlot(EEquipmentSlotType EquipmentSlotType);
	
	static bool IsSameEquipState(EEquipmentSlotType EquipmentSlotType, EEquipState WeaponEquipState);
	static bool IsSameWeaponHandType(EEquipmentSlotType EquipmentSlotType, EWeaponHandType WeaponHandType);
	static bool IsSameArmorType(EEquipmentSlotType EquipmentSlotType, EArmorType ArmorType);
	
	static bool IsPrimaryWeaponSlot(EEquipmentSlotType EquipmentSlotType);
	static bool IsSecondaryWeaponSlot(EEquipmentSlotType EquipmentSlotType);

	const UPanItemInstance* FindPairItemInstance(const UPanItemInstance* InBaseItemInstance, EEquipmentSlotType& OutEquipmentSlotType) const;
	
	bool IsAllEmpty(EEquipState EquipState) const;

	ALyraCharacter* GetCharacter() const;
	ALyraPlayerController* GetPlayerController() const;
	UPanEquipManagerComponent* GetEquipManager() const;
	
	UPanItemInstance* GetItemInstance(EEquipmentSlotType EquipmentSlotType) const;
	int32 GetItemCount(EEquipmentSlotType EquipmentSlotType) const;
	
	const TArray<FPanEquipmentEntry>& GetAllEntries() const;
	void GetAllWeaponItemInstances(TArray<UPanItemInstance*>& OutItemInstances) const;

public:
	// 장비 엔트리 변화 시점
	FOnEquipmentEntryChanged OnEquipmentEntryChanged;
	
private:
	friend class UPanItemManagerComponent;
	friend class ULyraCheatManager;
	
	// 장비 리스트
	UPROPERTY(Replicated)
	FPanEquipmentList EquipmentList;
};
