#pragma once

#include "PanDefine.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "PanEquipManagerComponent.generated.h"

class APanPocketWorldAttachment;
class ALyraCharacter;
class ALyraPlayerController;
class APanEquipmentBase;
class UPanItemInstance;
class UPanEquipManagerComponent;
class UPanEquipmentManagerComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEquipStateChanged, EEquipState, EEquipState);

USTRUCT(BlueprintType)
struct FPanEquipEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
private:
	void Init(UPanItemInstance* InItemInstance);
	
	void Equip();
	void Unequip();

public:
	UPanItemInstance* GetItemInstance() const { return ItemInstance; }

	void SetEquipmentActor(APanEquipmentBase* InEquipmentActor) { SpawnedEquipmentActor = InEquipmentActor; }
	APanEquipmentBase* GetEquipmentActor() const { return SpawnedEquipmentActor; }
	
private:
	friend struct FPanEquipList;
	friend class UPanEquipManagerComponent;
	
	UPROPERTY()
	TObjectPtr<UPanItemInstance> ItemInstance;

private:
	UPROPERTY(NotReplicated)
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

private:
	UPROPERTY(NotReplicated)
	TObjectPtr<APanEquipmentBase> SpawnedEquipmentActor;

	UPROPERTY(NotReplicated)
	TObjectPtr<APanEquipmentBase> SpawnedPocketWorldActor;

private:
	UPROPERTY(NotReplicated)
	FLyraAbilitySet_GrantedHandles BaseAbilitySetHandles;

	UPROPERTY(NotReplicated)
	FActiveGameplayEffectHandle BaseStatHandle;

public:
	UPROPERTY(NotReplicated)
	TObjectPtr<UPanEquipManagerComponent> EquipManager;
};

USTRUCT(BlueprintType)
struct FPanEquipList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FPanEquipList() : EquipManager(nullptr) { }
	FPanEquipList(UPanEquipManagerComponent* InOwnerComponent) : EquipManager(InOwnerComponent) { }

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

private:
	void Equip(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance);
	void Unequip(EEquipmentSlotType EquipmentSlotType);
	
public:
	TArray<FPanEquipEntry>& GetAllEntries() { return Entries; }
	
private:
	friend class UPanEquipManagerComponent;

	UPROPERTY()
	TArray<FPanEquipEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UPanEquipManagerComponent> EquipManager;
};

template<>
struct TStructOpsTypeTraits<FPanEquipList> : public TStructOpsTypeTraitsBase2<FPanEquipList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

UCLASS(BlueprintType)
class UPanEquipManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
	UPanEquipManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	
public:
	void Equip(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance);
	void Unequip(EEquipmentSlotType EquipmentSlotType);

	void EquipCurrentSlots();
	void UnequipCurrentSlots();
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ChangeEquipState(EEquipState NewEquipState);

	UFUNCTION(BlueprintCallable)
	bool CanChangeEquipState(EEquipState NewEquipState) const;

private:
	UFUNCTION()
	void OnRep_CurrentEquipState(EEquipState PrevEquipState);

	void BroadcastChangedMessage(EEquipState PrevEquipState, EEquipState NewEquipState);

public:
	ALyraCharacter* GetCharacter() const;
	ALyraPlayerController* GetPlayerController() const;
	
	TArray<FPanEquipEntry>& GetAllEntries();
	UAbilitySystemComponent* GetAbilitySystemComponent() const;
	UPanEquipmentManagerComponent* GetEquipmentManager() const;

	static EEquipmentSlotType ConvertToEquipmentSlotType(EWeaponHandType WeaponHandType, EEquipState EquipState);
	static EEquipmentSlotType ConvertToEquipmentSlotType(EWeaponHandType WeaponHandType, EWeaponSlotType WeaponSlotType);
	static EEquipmentSlotType ConvertToEquipmentSlotType(EArmorType ArmorType);
	static EEquipmentSlotType ConvertToEquipmentSlotType(EUtilitySlotType UtilitySlotType);
	
	static EWeaponSlotType ConvertToWeaponSlotType(EEquipmentSlotType EquipmentSlotType);
	static EWeaponHandType ConvertToWeaponHandType(EEquipmentSlotType EquipmentSlotType);
	static EArmorType ConvertToArmorType(EEquipmentSlotType EquipmentSlotType);
	static EUtilitySlotType ConvertToUtilitySlotType(EEquipmentSlotType EquipmentSlotType);
	static EEquipState ConvertToEquipState(EWeaponSlotType WeaponSlotType);
	static EEquipState ConvertToEquipState(EUtilitySlotType UtilitySlotType);

	static bool IsWeaponEquipState(EEquipState EquipState);
	static bool IsUtilityEquipState(EEquipState EquipState);

	static const TArray<EEquipmentSlotType>& GetEquipmentSlotsByEquipState(EEquipState EquipState);
	
	UFUNCTION(BlueprintCallable)
	void ChangeShouldHiddenEquipments(bool bNewShouldHiddenEquipments);
	
	bool ShouldHiddenEquipments() const { return bShouldHiddenEquipments; }
	EEquipState GetCurrentEquipState() const { return CurrentEquipState; }
	
	APanEquipmentBase* GetFirstEquippedActor() const;
	APanEquipmentBase* GetEquippedActor(EWeaponHandType WeaponHandType) const;
	void GetAllEquippedActors(TArray<APanEquipmentBase*>& OutActors) const;

	UPanItemInstance* GetFirstEquippedItemInstance(bool bIgnoreArmor = true) const;
	UPanItemInstance* GetEquippedItemInstance(EArmorType ArmorType) const;
	UPanItemInstance* GetEquippedItemInstance(EWeaponHandType WeaponHandType) const;
	UPanItemInstance* GetEquippedItemInstance(EEquipmentSlotType EquipmentSlotType) const;

public:
	FOnEquipStateChanged OnEquipStateChanged;
	
private:
	UPROPERTY(Replicated)
	FPanEquipList EquipList;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentEquipState)
	EEquipState CurrentEquipState = EEquipState::Count;

	UPROPERTY(Replicated)
	bool bShouldHiddenEquipments = false;
};
