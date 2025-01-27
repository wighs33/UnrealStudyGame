#include "PanEquipManagerComponent.h"

#include "AbilitySystemGlobals.h"
#include "PanCosmeticManagerComponent.h"
#include "PanEquipmentManagerComponent.h"
#include "LyraGameplayTags.h"
#include "Engine/ActorChannel.h"
#include "Item/PanItemInstance.h"
#include "Item/Fragments/PanItemFragment_Equipable.h"
#include "Item/Fragments/PanItemFragment_Equipable_Armor.h"
#include "Item/Fragments/PanItemFragment_Equipable_Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Actors/PanEquipmentBase.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
//#include "AbilitySystem/Attributes/PanCombatSet.h"
#include "Character/LyraCharacter.h"
#include "Player/LyraPlayerController.h"
//#include "PocketWorld/PanPocketStage.h"
//#include "PocketWorld/PanPocketWorldSubsystem.h"
#include "System/PanGameplayTagStack.h"
#include "System/LyraAssetManager.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanEquipManagerComponent)

void FPanEquipEntry::Init(UPanItemInstance* InItemInstance)
{
	if (ItemInstance == InItemInstance)
		return;

	ItemInstance = InItemInstance;
	ItemInstance ? Equip() : Unequip();
}

void FPanEquipEntry::Equip()
{
	if (ItemInstance == nullptr)
		return;

	ALyraCharacter* Character = EquipManager->GetCharacter();
	if (Character == nullptr)
		return;

	const UPanItemFragment_Equipable* EquippableFragment = ItemInstance->FindFragmentByClass<UPanItemFragment_Equipable>();
	if (EquippableFragment == nullptr)
		return;

	if (EquipManager->GetOwner()->HasAuthority())
	{
		/* - TEMP Rookiss
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(EquipManager->GetAbilitySystemComponent()))
		{
			// Remove Previous Ability
			BaseAbilitySetHandles.TakeFromAbilitySystem(ASC);
	
			// Add Current Ability
			if (const ULyraAbilitySet* BaseAbilitySet = EquippableFragment->BaseAbilitySet)
			{
				BaseAbilitySet->GiveToAbilitySystem(ASC, &BaseAbilitySetHandles, ItemInstance);
			}

			// Remove Previous Stat
			ASC->RemoveActiveGameplayEffect(BaseStatHandle);
			BaseStatHandle.Invalidate();
	
			// Add Current Stat
			const TSubclassOf<UGameplayEffect> AttributeModifierGE = ULyraAssetManager::GetSubclassByPath(ULyraGameData::Get().AttributeModifierGameplayEffect);
			check(AttributeModifierGE);
		
			const FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AttributeModifierGE, 1.f, ContextHandle);
			const TSharedPtr<FGameplayEffectSpec>& SpecData = SpecHandle.Data;
			
			for (const FGameplayModifierInfo& ModifierInfo : SpecData->Def->Modifiers)
			{
				SpecData->SetSetByCallerMagnitude(ModifierInfo.ModifierMagnitude.GetSetByCallerFloat().DataTag, 0);
			}
			
			for (const FPanGameplayTagStack& Stack : ItemInstance->GetStatContainer().GetStacks())
			{
				SpecData->SetSetByCallerMagnitude(Stack.GetStackTag(), Stack.GetStackCount());
			}
			
			BaseStatHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
		}
		*/

		if (EquippableFragment->EquipmentType == EEquipmentType::Weapon || EquippableFragment->EquipmentType == EEquipmentType::Utility)
		{
			// Despawn Previous Real Weapon
			if (IsValid(SpawnedEquipmentActor))
			{
				SpawnedEquipmentActor->Destroy();
			}

			// Spawn Current Real Weapon
			const UPanItemFragment_Equipable_Attachment* AttachmentFragment = ItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Attachment>();
			const FPanWeaponAttachInfo& AttachInfo = AttachmentFragment->WeaponAttachInfo;
			if (AttachInfo.SpawnWeaponClass)
			{
				UWorld* World = EquipManager->GetWorld();
				APanEquipmentBase* NewWeaponActor = World->SpawnActorDeferred<APanEquipmentBase>(AttachInfo.SpawnWeaponClass, FTransform::Identity, Character);
				NewWeaponActor->Init(ItemInstance->GetItemTemplateID(), EquipmentSlotType);
				NewWeaponActor->SetActorRelativeTransform(AttachInfo.AttachTransform);
				NewWeaponActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, AttachInfo.AttachSocket);
				NewWeaponActor->SetActorHiddenInGame(EquipManager->ShouldHiddenEquipments());
				NewWeaponActor->FinishSpawning(FTransform::Identity, true);
			}
		}
	}
	else
	{
		if (EquippableFragment->EquipmentType == EEquipmentType::Weapon || EquippableFragment->EquipmentType == EEquipmentType::Utility)
		{
			if (Character->IsLocallyControlled())
			{
				// Despawn Previous Pocket Weapon
				if (IsValid(SpawnedPocketWorldActor))
				{
					SpawnedPocketWorldActor->Destroy();
				}

				/* TEMP Rookiss
				// Spawn Current Pocket Weapon				
				const UPanItemFragment_Equipable_Attachment* AttachmentFragment = ItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Attachment>();
				if (UPanPocketWorldSubsystem* PocketWorldSubsystem = EquipManager->GetWorld()->GetSubsystem<UPanPocketWorldSubsystem>())
				{
					if (APlayerController* PC = Character->GetLyraPlayerController())
					{
						PocketWorldSubsystem->RegisterAndCallForGetPocketStage(PC->GetLocalPlayer(),
							FGetPocketStageDelegate::CreateLambda([this, AttachmentFragment](APanPocketStage* PocketStage)
							{
								if (IsValid(PocketStage))
								{
									ACharacter* Character = PocketStage->GetCharacter();
									const FPanWeaponAttachInfo& AttachInfo = AttachmentFragment->WeaponAttachInfo;
									
									UWorld* World = EquipManager->GetWorld();
									SpawnedPocketWorldActor = World->SpawnActorDeferred<APanEquipmentBase>(AttachInfo.SpawnWeaponClass, FTransform::Identity, Character);
									SpawnedPocketWorldActor->SetActorRelativeTransform(AttachInfo.AttachTransform);
									SpawnedPocketWorldActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, AttachInfo.AttachSocket);
									SpawnedPocketWorldActor->bOnlyUseForLocal = true;
									SpawnedPocketWorldActor->FinishSpawning(FTransform::Identity, true);

									PocketStage->RefreshLightingChannelToActors();
									
									UAnimMontage* PocketWorldIdleMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->PocketWorldIdleMontage);
									Character->PlayAnimMontage(PocketWorldIdleMontage);
								}
							})
						);
					}
				}*/
			}
		}
		else if (EquippableFragment->EquipmentType == EEquipmentType::Armor)
		{
			// Refresh Real Armor Mesh
			const UPanItemFragment_Equipable_Armor* ArmorFragment = ItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Armor>();
			if (UPanCosmeticManagerComponent* CharacterCosmetics = Character->FindComponentByClass<UPanCosmeticManagerComponent>())
			{
				CharacterCosmetics->RefreshArmorMesh(ArmorFragment->ArmorType, ArmorFragment);
			}

			// Refresh Pocket Armor Mesh
			if (Character->IsLocallyControlled())
			{
				/* TEMP Rookiss				
				if (UPanPocketWorldSubsystem* PocketWorldSubsystem = EquipManager->GetWorld()->GetSubsystem<UPanPocketWorldSubsystem>())
				{
					if (APlayerController* PC = Character->GetLyraPlayerController())
					{
						PocketWorldSubsystem->RegisterAndCallForGetPocketStage(PC->GetLocalPlayer(),
							FGetPocketStageDelegate::CreateLambda([ArmorFragment](APanPocketStage* PocketStage)
							{
								if (IsValid(PocketStage))
								{
									if (UPanCosmeticManagerComponent* CosmeticManager = PocketStage->GetCosmeticManager())
									{
										CosmeticManager->RefreshArmorMesh(ArmorFragment->ArmorType, ArmorFragment);
									}
								}
							})
						);
					}
				}*/
			}
		}
	}

	if (EquippableFragment->EquipmentType == EEquipmentType::Weapon || EquippableFragment->EquipmentType == EEquipmentType::Utility)
	{
		const UPanItemFragment_Equipable_Attachment* AttachmentFragment = ItemInstance->FindFragmentByClass<UPanItemFragment_Equipable_Attachment>();
		if (USkeletalMeshComponent* MeshComponent = Character->GetMesh())
		{
			if (AttachmentFragment->AnimInstanceClass)
			{
				MeshComponent->LinkAnimClassLayers(AttachmentFragment->AnimInstanceClass);
			}

			UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
			if (ASC && ASC->HasMatchingGameplayTag(LyraGameplayTags::Status_Interact) == false)
			{
				UAnimMontage* EquipMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->EquipMontage);
				if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
				{
					if (AnimInstance->GetCurrentActiveMontage() != EquipMontage)
					{
						Character->PlayAnimMontage(EquipMontage);
					}
				}
			}
		}
	}
}

void FPanEquipEntry::Unequip()
{
	if (EquipManager->GetOwner()->HasAuthority())
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(EquipManager->GetAbilitySystemComponent()))
		{
			// Remove Ability
			BaseAbilitySetHandles.TakeFromAbilitySystem(ASC);

			// Remove Stat
			ASC->RemoveActiveGameplayEffect(BaseStatHandle);
			BaseStatHandle.Invalidate();
		}
		
		// Despawn Real Weapon
		if (UPanEquipmentManagerComponent::IsWeaponSlot(EquipmentSlotType) || UPanEquipmentManagerComponent::IsUtilitySlot(EquipmentSlotType))
		{
			if (IsValid(SpawnedEquipmentActor))
			{
				SpawnedEquipmentActor->Destroy();
			}
		}
	}
	else
	{
		if (ALyraCharacter* Character = EquipManager->GetCharacter())
		{
			if (UPanEquipmentManagerComponent::IsWeaponSlot(EquipmentSlotType) || UPanEquipmentManagerComponent::IsUtilitySlot(EquipmentSlotType))
			{
				// Despawn Pocket Weapon
				if (Character->IsLocallyControlled())
				{
					/* TEMP Rookiss
					
					if (UPanPocketWorldSubsystem* PocketWorldSubsystem = EquipManager->GetWorld()->GetSubsystem<UPanPocketWorldSubsystem>())
					{
						if (APlayerController* PC = Character->GetLyraPlayerController())
						{
							PocketWorldSubsystem->RegisterAndCallForGetPocketStage(PC->GetLocalPlayer(),
								FGetPocketStageDelegate::CreateLambda([this](APanPocketStage* PocketStage)
								{
									if (IsValid(PocketStage))
									{
										if (IsValid(SpawnedPocketWorldActor))
										{
											SpawnedPocketWorldActor->Destroy();
										}
									}
								})
							);
						}
					}*/
				}
			}
			else if (UPanEquipmentManagerComponent::IsArmorSlot(EquipmentSlotType))
			{
				// Refresh Real Armor Mesh
				EArmorType ArmorType = EquipManager->ConvertToArmorType(EquipmentSlotType);
		
				if (UPanCosmeticManagerComponent* CharacterCosmetics = Character->FindComponentByClass<UPanCosmeticManagerComponent>())
				{
					CharacterCosmetics->RefreshArmorMesh(ArmorType, nullptr);
				}
				
				// Refresh Pocket Armor Mesh
				if (Character->IsLocallyControlled())
				{
					/* TEMP Rookiss
					
					if (UPanPocketWorldSubsystem* PocketWorldSubsystem = EquipManager->GetWorld()->GetSubsystem<UPanPocketWorldSubsystem>())
					{
						if (APlayerController* PC = Character->GetLyraPlayerController())
						{
							PocketWorldSubsystem->RegisterAndCallForGetPocketStage(PC->GetLocalPlayer(),
								FGetPocketStageDelegate::CreateLambda([ArmorType](APanPocketStage* PocketStage)
								{
									if (IsValid(PocketStage))
									{
										if (UPanCosmeticManagerComponent* CosmeticManager = PocketStage->GetCosmeticManager())
										{
											CosmeticManager->RefreshArmorMesh(ArmorType, nullptr);
										}
									}
								})
							);
						}
					}*/
				}
			}
		}
	}
}

bool FPanEquipList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FPanEquipEntry, FPanEquipList>(Entries, DeltaParams,*this);
}

void FPanEquipList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 AddedIndex : AddedIndices)
	{
		FPanEquipEntry& Entry = Entries[AddedIndex];
		Entry.EquipManager = EquipManager;
		Entry.EquipmentSlotType = (EEquipmentSlotType)AddedIndex;
		
		if (Entry.GetItemInstance())
		{
			Entry.Equip();
		}
	}
}

void FPanEquipList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 ChangedIndex : ChangedIndices)
	{
		FPanEquipEntry& Entry = Entries[ChangedIndex];
		Entry.GetItemInstance() ? Entry.Equip() : Entry.Unequip();
	}
}

void FPanEquipList::Equip(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance)
{
	FPanEquipEntry& Entry = Entries[(int32)(EquipmentSlotType)];
	Entry.Init(ItemInstance);
	MarkItemDirty(Entry);
}

void FPanEquipList::Unequip(EEquipmentSlotType EquipmentSlotType)
{
	FPanEquipEntry& Entry = Entries[(int32)(EquipmentSlotType)];
	Entry.Init(nullptr);
	MarkItemDirty(Entry);
}

UPanEquipManagerComponent::UPanEquipManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipList(this)
{
    SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UPanEquipManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<FPanEquipEntry>& Entries = EquipList.Entries;
		Entries.SetNum((int32)EEquipmentSlotType::Count);

		for (int32 i = 0; i < Entries.Num(); i++)
		{
			FPanEquipEntry& Entry = Entries[i];
			Entry.EquipManager = this;
			Entry.EquipmentSlotType = (EEquipmentSlotType)i;
			EquipList.MarkItemDirty(Entry);
		}
	}
}

void UPanEquipManagerComponent::UninitializeComponent()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
		{
			Unequip((EEquipmentSlotType)i);
		}
	}
	
	Super::UninitializeComponent();
}

void UPanEquipManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipList);
	DOREPLIFETIME(ThisClass, CurrentEquipState);
	DOREPLIFETIME(ThisClass, bShouldHiddenEquipments);
}

bool UPanEquipManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (FPanEquipEntry& Entry : EquipList.Entries)
	{
		UPanItemInstance* ItemInstance = Entry.GetItemInstance();
		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}
	
	return bWroteSomething;
}

void UPanEquipManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FPanEquipEntry& Entry : EquipList.Entries)
		{
			UPanItemInstance* ItemInstance = Entry.GetItemInstance();
			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

void UPanEquipManagerComponent::Equip(EEquipmentSlotType EquipmentSlotType, UPanItemInstance* ItemInstance)
{
	check(GetOwner()->HasAuthority());

	if (EquipmentSlotType == EEquipmentSlotType::Count || ItemInstance == nullptr)
		return;
	
	EquipList.Equip(EquipmentSlotType, ItemInstance);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

void UPanEquipManagerComponent::Unequip(EEquipmentSlotType EquipmentSlotType)
{
	check(GetOwner()->HasAuthority())

	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return;
	
	TArray<FPanEquipEntry>& Entries = EquipList.Entries;
	FPanEquipEntry& Entry = Entries[(int32)EquipmentSlotType];
	UPanItemInstance* RemovedItemInstance = Entry.GetItemInstance();
	
	EquipList.Unequip(EquipmentSlotType);
	if (IsUsingRegisteredSubObjectList() && RemovedItemInstance)
	{
		RemoveReplicatedSubObject(RemovedItemInstance);
	}
}

void UPanEquipManagerComponent::EquipCurrentSlots()
{
	check(GetOwner()->HasAuthority());

	if (CurrentEquipState == EEquipState::Count)
		return;
	
	if (UPanEquipmentManagerComponent* EquipmentManager = GetEquipmentManager())
	{
		for (EEquipmentSlotType EquipmentSlotType : UPanEquipManagerComponent::GetEquipmentSlotsByEquipState(CurrentEquipState))
		{
			Equip(EquipmentSlotType, EquipmentManager->GetItemInstance(EquipmentSlotType));
		}
	}
}

void UPanEquipManagerComponent::UnequipCurrentSlots()
{
	check(GetOwner()->HasAuthority());

	if (CurrentEquipState == EEquipState::Count)
		return;
	
	for (EEquipmentSlotType EquipmentSlotType : UPanEquipManagerComponent::GetEquipmentSlotsByEquipState(CurrentEquipState))
	{
		Unequip(EquipmentSlotType);
	}
}

void UPanEquipManagerComponent::ChangeEquipState(EEquipState NewEquipState)
{
	check(GetOwner()->HasAuthority());

	if (CanChangeEquipState(NewEquipState))
	{
		if (CurrentEquipState == NewEquipState)
		{
			NewEquipState = EEquipState::Unarmed;
		}
		
		UnequipCurrentSlots();
		CurrentEquipState = NewEquipState;
		EquipCurrentSlots();
	}
}

bool UPanEquipManagerComponent::CanChangeEquipState(EEquipState NewEquipState) const
{
	if (NewEquipState == EEquipState::Count)
		return false;

	if (CurrentEquipState == EEquipState::Unarmed && NewEquipState == EEquipState::Unarmed)
		return false;

	if (CurrentEquipState == NewEquipState)
		return true;
	
	UPanEquipmentManagerComponent* EquipmentManager = GetEquipmentManager();
	if (EquipmentManager == nullptr)
		return false;
	
	return (EquipmentManager->IsAllEmpty(NewEquipState) == false);
}

APanEquipmentBase* UPanEquipManagerComponent::GetFirstEquippedActor() const
{
	APanEquipmentBase* EquipmentActor = nullptr;
	const TArray<FPanEquipEntry>& Entries = EquipList.Entries;

	if (IsWeaponEquipState(CurrentEquipState))
	{
		for (int i = 0; i < (int32)EWeaponHandType::Count; i++)
		{
			const int32 EntryIndex = (int32)ConvertToEquipmentSlotType((EWeaponHandType)i, CurrentEquipState);
			if (Entries.IsValidIndex(EntryIndex) == false)
				continue;

			EquipmentActor = Entries[EntryIndex].GetEquipmentActor();
			if (EquipmentActor)
				break;
		}
	}
	else if (IsUtilityEquipState(CurrentEquipState))
	{
		const int32 EntryIndex = (int32)ConvertToEquipmentSlotType(EWeaponHandType::Count, CurrentEquipState);
		if (Entries.IsValidIndex(EntryIndex))
		{
			EquipmentActor = Entries[EntryIndex].GetEquipmentActor();
		}
	}
	
	return EquipmentActor;
}

APanEquipmentBase* UPanEquipManagerComponent::GetEquippedActor(EWeaponHandType WeaponHandType) const
{
	if (WeaponHandType == EWeaponHandType::Count)
		return GetFirstEquippedActor();
	
	const TArray<FPanEquipEntry>& Entries = EquipList.Entries;
	const int32 EntryIndex = (int32)ConvertToEquipmentSlotType(WeaponHandType, CurrentEquipState);
	return Entries.IsValidIndex(EntryIndex) ? Entries[EntryIndex].GetEquipmentActor() : nullptr;
}

void UPanEquipManagerComponent::GetAllEquippedActors(TArray<APanEquipmentBase*>& OutActors) const
{
	OutActors.Reset();
	
	const TArray<FPanEquipEntry>& Entries = EquipList.Entries;
	
	if (IsWeaponEquipState(CurrentEquipState))
	{
		for (int32 i = 0; i < (int32)EWeaponHandType::Count; i++)
		{
			const int32 EntryIndex = (int32)ConvertToEquipmentSlotType((EWeaponHandType)i, CurrentEquipState);
			if (Entries.IsValidIndex(EntryIndex) && Entries[EntryIndex].GetEquipmentActor())
			{
				OutActors.Add(Entries[EntryIndex].GetEquipmentActor());
			}
		}
	}
	else if (IsUtilityEquipState(CurrentEquipState))
	{
		const int32 EntryIndex = (int32)ConvertToEquipmentSlotType(EWeaponHandType::Count, CurrentEquipState);
		if (Entries.IsValidIndex(EntryIndex) && Entries[EntryIndex].GetEquipmentActor())
		{
			OutActors.Add(Entries[EntryIndex].GetEquipmentActor());
		}
	}
}

UPanItemInstance* UPanEquipManagerComponent::GetFirstEquippedItemInstance(bool bIgnoreArmor) const
{
	const TArray<FPanEquipEntry>& Entries = EquipList.Entries;

	if (bIgnoreArmor == false)
	{
		for (int i = 0; i < (int32)EArmorType::Count; i++)
		{
			const int32 EntryIndex = (int32)ConvertToEquipmentSlotType((EArmorType)i);
			if (Entries.IsValidIndex(EntryIndex) == false)
				continue;

			if (UPanItemInstance* ItemInstance = Entries[EntryIndex].GetItemInstance())
				return ItemInstance;
		}
	}

	UPanItemInstance* ItemInstance = nullptr;
	
	if (IsWeaponEquipState(CurrentEquipState))
	{
		for (int i = 0; i < (int32)EWeaponHandType::Count; i++)
		{
			const int32 EntryIndex = (int32)ConvertToEquipmentSlotType((EWeaponHandType)i, CurrentEquipState);
			if (Entries.IsValidIndex(EntryIndex) == false)
				continue;

			ItemInstance = Entries[EntryIndex].GetItemInstance();
			if (ItemInstance)
				break;
		}
	}
	else if (IsUtilityEquipState(CurrentEquipState))
	{
		const int32 EntryIndex = (int32)ConvertToEquipmentSlotType(EWeaponHandType::Count, CurrentEquipState);
		if (Entries.IsValidIndex(EntryIndex))
		{
			ItemInstance = Entries[EntryIndex].GetItemInstance();
		}
	}
	
	return ItemInstance;
}

UPanItemInstance* UPanEquipManagerComponent::GetEquippedItemInstance(EArmorType ArmorType) const
{
	const TArray<FPanEquipEntry>& Entries = EquipList.Entries;
	const int32 EntryIndex = (int32)ConvertToEquipmentSlotType(ArmorType);
	return Entries.IsValidIndex(EntryIndex) ? Entries[EntryIndex].GetItemInstance() : nullptr;
}

UPanItemInstance* UPanEquipManagerComponent::GetEquippedItemInstance(EWeaponHandType WeaponHandType) const
{
	if (WeaponHandType == EWeaponHandType::Count)
		return GetFirstEquippedItemInstance();
	
	const TArray<FPanEquipEntry>& Entries = EquipList.Entries;
	const int32 EntryIndex = (int32)ConvertToEquipmentSlotType(WeaponHandType, CurrentEquipState);
	return Entries.IsValidIndex(EntryIndex) ? Entries[EntryIndex].GetItemInstance() : nullptr;
}

UPanItemInstance* UPanEquipManagerComponent::GetEquippedItemInstance(EEquipmentSlotType EquipmentSlotType) const
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return nullptr;

	const TArray<FPanEquipEntry>& Entries = EquipList.Entries;
	const int32 EntryIndex = (int32)EquipmentSlotType;
	return Entries.IsValidIndex(EntryIndex) ? Entries[EntryIndex].GetItemInstance() : nullptr;
}

void UPanEquipManagerComponent::OnRep_CurrentEquipState(EEquipState PrevEquipState)
{
	BroadcastChangedMessage(PrevEquipState, CurrentEquipState);
}

void UPanEquipManagerComponent::BroadcastChangedMessage(EEquipState PrevEquipState, EEquipState NewEquipState)
{
	if (OnEquipStateChanged.IsBound())
	{
		OnEquipStateChanged.Broadcast(PrevEquipState, NewEquipState);
	}
}

ALyraCharacter* UPanEquipManagerComponent::GetCharacter() const
{
	return Cast<ALyraCharacter>(GetOwner());
}

ALyraPlayerController* UPanEquipManagerComponent::GetPlayerController() const
{
	if (ALyraCharacter* LyraCharacter = GetCharacter())
	{
		return LyraCharacter->GetLyraPlayerController();
	}
	return nullptr;
}

TArray<FPanEquipEntry>& UPanEquipManagerComponent::GetAllEntries()
{
	return EquipList.GetAllEntries();
}

UAbilitySystemComponent* UPanEquipManagerComponent::GetAbilitySystemComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

UPanEquipmentManagerComponent* UPanEquipManagerComponent::GetEquipmentManager() const
{
	if (ALyraCharacter* LyraCharacter = GetCharacter())
	{
		return LyraCharacter->FindComponentByClass<UPanEquipmentManagerComponent>();
	}
	return nullptr;
}

EEquipmentSlotType UPanEquipManagerComponent::ConvertToEquipmentSlotType(EWeaponHandType WeaponHandType, EEquipState EquipState)
{
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	if (EquipState == EEquipState::Unarmed)
	{
		switch (WeaponHandType)
		{
		case EWeaponHandType::LeftHand:  EquipmentSlotType = EEquipmentSlotType::Unarmed_LeftHand;  break;
		case EWeaponHandType::RightHand: EquipmentSlotType = EEquipmentSlotType::Unarmed_RightHand; break;
		}
	}
	else if (EquipState == EEquipState::Weapon_Primary)
	{
		switch (WeaponHandType)
		{
		case EWeaponHandType::LeftHand:  EquipmentSlotType = EEquipmentSlotType::Primary_LeftHand;  break;
		case EWeaponHandType::RightHand: EquipmentSlotType = EEquipmentSlotType::Primary_RightHand; break;
		case EWeaponHandType::TwoHand:   EquipmentSlotType = EEquipmentSlotType::Primary_TwoHand;   break;
		}
	}
	else if (EquipState == EEquipState::Weapon_Secondary)
	{
		switch (WeaponHandType)
		{
		case EWeaponHandType::LeftHand:  EquipmentSlotType = EEquipmentSlotType::Secondary_LeftHand;  break;
		case EWeaponHandType::RightHand: EquipmentSlotType = EEquipmentSlotType::Secondary_RightHand; break;
		case EWeaponHandType::TwoHand:   EquipmentSlotType = EEquipmentSlotType::Secondary_TwoHand;   break;
		}
	}
	else if (EquipState == EEquipState::Utility_Primary)
	{
		EquipmentSlotType = EEquipmentSlotType::Utility_Primary;
	}
	else if (EquipState == EEquipState::Utility_Secondary)
	{
		EquipmentSlotType = EEquipmentSlotType::Utility_Secondary;
	}
	else if (EquipState == EEquipState::Utility_Tertiary)
	{
		EquipmentSlotType = EEquipmentSlotType::Utility_Tertiary;
	}
	else if (EquipState == EEquipState::Utility_Quaternary)
	{
		EquipmentSlotType = EEquipmentSlotType::Utility_Quaternary;
	}
	
	return EquipmentSlotType;
}

EEquipmentSlotType UPanEquipManagerComponent::ConvertToEquipmentSlotType(EWeaponHandType WeaponHandType, EWeaponSlotType WeaponSlotType)
{
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	if (WeaponSlotType == EWeaponSlotType::Primary)
	{
		switch (WeaponHandType)
		{
		case EWeaponHandType::LeftHand:  EquipmentSlotType = EEquipmentSlotType::Primary_LeftHand;  break;
		case EWeaponHandType::RightHand: EquipmentSlotType = EEquipmentSlotType::Primary_RightHand; break;
		case EWeaponHandType::TwoHand:   EquipmentSlotType = EEquipmentSlotType::Primary_TwoHand;   break;
		}
	}
	else if (WeaponSlotType == EWeaponSlotType::Secondary)
	{
		switch (WeaponHandType)
		{
		case EWeaponHandType::LeftHand:  EquipmentSlotType = EEquipmentSlotType::Secondary_LeftHand;  break;
		case EWeaponHandType::RightHand: EquipmentSlotType = EEquipmentSlotType::Secondary_RightHand; break;
		case EWeaponHandType::TwoHand:   EquipmentSlotType = EEquipmentSlotType::Secondary_TwoHand;   break;
		}
	}
	
	return EquipmentSlotType;
}

EEquipmentSlotType UPanEquipManagerComponent::ConvertToEquipmentSlotType(EArmorType ArmorType)
{
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	switch (ArmorType)
	{
	case EArmorType::Helmet: EquipmentSlotType = EEquipmentSlotType::Helmet; break;
	case EArmorType::Chest:  EquipmentSlotType = EEquipmentSlotType::Chest;  break;
	case EArmorType::Legs:   EquipmentSlotType = EEquipmentSlotType::Legs;   break;
	case EArmorType::Hands:  EquipmentSlotType = EEquipmentSlotType::Hands;  break;
	case EArmorType::Foot:   EquipmentSlotType = EEquipmentSlotType::Foot;   break;
	}

	return EquipmentSlotType;
}

EEquipmentSlotType UPanEquipManagerComponent::ConvertToEquipmentSlotType(EUtilitySlotType UtilitySlotType)
{
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	switch (UtilitySlotType)
	{
	case EUtilitySlotType::Primary:		EquipmentSlotType = EEquipmentSlotType::Utility_Primary;	break;
	case EUtilitySlotType::Secondary:	EquipmentSlotType = EEquipmentSlotType::Utility_Secondary;	break;
	case EUtilitySlotType::Tertiary:	EquipmentSlotType = EEquipmentSlotType::Utility_Tertiary;	break;
	case EUtilitySlotType::Quaternary:	EquipmentSlotType = EEquipmentSlotType::Utility_Quaternary;	break;
	}
	
	return EquipmentSlotType;
}

EWeaponHandType UPanEquipManagerComponent::ConvertToWeaponHandType(EEquipmentSlotType EquipmentSlotType)
{
	EWeaponHandType WeaponHandType = EWeaponHandType::Count;
	
	switch (EquipmentSlotType)
	{
	case EEquipmentSlotType::Unarmed_LeftHand:
	case EEquipmentSlotType::Primary_LeftHand:
	case EEquipmentSlotType::Secondary_LeftHand:
		WeaponHandType = EWeaponHandType::LeftHand;
		break;
	case EEquipmentSlotType::Unarmed_RightHand:
	case EEquipmentSlotType::Primary_RightHand:
	case EEquipmentSlotType::Secondary_RightHand:
		WeaponHandType = EWeaponHandType::RightHand;
		break;
	case EEquipmentSlotType::Primary_TwoHand:
	case EEquipmentSlotType::Secondary_TwoHand:
		WeaponHandType = EWeaponHandType::TwoHand;
		break;
	}

	return WeaponHandType;
}

EArmorType UPanEquipManagerComponent::ConvertToArmorType(EEquipmentSlotType EquipmentSlotType)
{
	EArmorType ArmorType = EArmorType::Count;
	
	switch (EquipmentSlotType)
	{
	case EEquipmentSlotType::Helmet:	ArmorType = EArmorType::Helmet;	break;
	case EEquipmentSlotType::Chest:		ArmorType = EArmorType::Chest;	break;
	case EEquipmentSlotType::Legs:		ArmorType = EArmorType::Legs;	break;
	case EEquipmentSlotType::Hands:		ArmorType = EArmorType::Hands;	break;
	case EEquipmentSlotType::Foot:		ArmorType = EArmorType::Foot;	break;
	}

	return ArmorType;
}

EUtilitySlotType UPanEquipManagerComponent::ConvertToUtilitySlotType(EEquipmentSlotType EquipmentSlotType)
{
	EUtilitySlotType UtilitySlotType = EUtilitySlotType::Count;

	switch (EquipmentSlotType)
	{
	case EEquipmentSlotType::Utility_Primary:		UtilitySlotType = EUtilitySlotType::Primary;	break;
	case EEquipmentSlotType::Utility_Secondary:		UtilitySlotType = EUtilitySlotType::Secondary;	break;
	case EEquipmentSlotType::Utility_Tertiary:		UtilitySlotType = EUtilitySlotType::Tertiary;	break;
	case EEquipmentSlotType::Utility_Quaternary:	UtilitySlotType = EUtilitySlotType::Quaternary;	break;
	}

	return UtilitySlotType;
}

EEquipState UPanEquipManagerComponent::ConvertToEquipState(EWeaponSlotType WeaponSlotType)
{
	EEquipState EquipState = EEquipState::Count;

	switch (WeaponSlotType)
	{
	case EWeaponSlotType::Primary:		EquipState = EEquipState::Weapon_Primary;		break;
	case EWeaponSlotType::Secondary:	EquipState = EEquipState::Weapon_Secondary;		break;
	}

	return EquipState;
}

EEquipState UPanEquipManagerComponent::ConvertToEquipState(EUtilitySlotType UtilitySlotType)
{
	EEquipState EquipState = EEquipState::Count;

	switch (UtilitySlotType)
	{
	case EUtilitySlotType::Primary:		EquipState = EEquipState::Utility_Primary;		break;
	case EUtilitySlotType::Secondary:	EquipState = EEquipState::Utility_Secondary;	break;
	case EUtilitySlotType::Tertiary:	EquipState = EEquipState::Utility_Tertiary;		break;
	case EUtilitySlotType::Quaternary:	EquipState = EEquipState::Utility_Quaternary;	break;
	}

	return EquipState;
}

bool UPanEquipManagerComponent::IsWeaponEquipState(EEquipState EquipState)
{
	return (EEquipState::Unarmed <= EquipState && EquipState <= EEquipState::Weapon_Secondary);
}

bool UPanEquipManagerComponent::IsUtilityEquipState(EEquipState EquipState)
{
	return (EEquipState::Utility_Primary <= EquipState && EquipState <= EEquipState::Utility_Quaternary);
}

const TArray<EEquipmentSlotType>& UPanEquipManagerComponent::GetEquipmentSlotsByEquipState(EEquipState EquipState)
{
	static const TArray<TArray<EEquipmentSlotType>> EquipmentSlotsByEquipState = {
		{ EEquipmentSlotType::Unarmed_LeftHand,    EEquipmentSlotType::Unarmed_RightHand                                            },
		{ EEquipmentSlotType::Primary_LeftHand,    EEquipmentSlotType::Primary_RightHand,    EEquipmentSlotType::Primary_TwoHand    },
		{ EEquipmentSlotType::Secondary_LeftHand,  EEquipmentSlotType::Secondary_RightHand,  EEquipmentSlotType::Secondary_TwoHand  },
		{ EEquipmentSlotType::Utility_Primary }, { EEquipmentSlotType::Utility_Secondary }, { EEquipmentSlotType::Utility_Tertiary }, { EEquipmentSlotType::Utility_Quaternary },
	};

	if (EquipmentSlotsByEquipState.IsValidIndex((int32)EquipState))
	{
		return EquipmentSlotsByEquipState[(int32)EquipState];
	}
	else
	{
		static const TArray<EEquipmentSlotType> EmptyEquipmentSlots;
		return EmptyEquipmentSlots;
	}
}

EWeaponSlotType UPanEquipManagerComponent::ConvertToWeaponSlotType(EEquipmentSlotType EquipmentSlotType)
{
	EWeaponSlotType WeaponSlotType = EWeaponSlotType::Count;

	switch (EquipmentSlotType)
	{
	case EEquipmentSlotType::Primary_LeftHand:
	case EEquipmentSlotType::Primary_RightHand:
	case EEquipmentSlotType::Primary_TwoHand:
		WeaponSlotType = EWeaponSlotType::Primary;
		break;
	case EEquipmentSlotType::Secondary_LeftHand:
	case EEquipmentSlotType::Secondary_RightHand:
	case EEquipmentSlotType::Secondary_TwoHand:
		WeaponSlotType = EWeaponSlotType::Secondary;
		break;
	}

	return WeaponSlotType;
}

void UPanEquipManagerComponent::ChangeShouldHiddenEquipments(bool bNewShouldHiddenEquipments)
{
	bShouldHiddenEquipments = bNewShouldHiddenEquipments;

	TArray<APanEquipmentBase*> OutEquippedActors;
	GetAllEquippedActors(OutEquippedActors);

	for (APanEquipmentBase* WeaponActor : OutEquippedActors)
	{
		if (IsValid(WeaponActor))
		{
			WeaponActor->SetActorHiddenInGame(bShouldHiddenEquipments);
		}
	}
}
