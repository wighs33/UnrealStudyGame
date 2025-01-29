#include "PanItemManagerComponent.h"

#include "PanEquipmentManagerComponent.h"
#include "PanInventoryManagerComponent.h"
//#include "Actors/PanPickupableItemBase.h"
#include "Components/CapsuleComponent.h"
#include "Data/PanItemData.h"
#include "GameFramework/Character.h"
#include "Item/PanItemInstance.h"
#include "Item/Fragments/PanItemFragment_Equipable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanItemManagerComponent)

UPanItemManagerComponent::UPanItemManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UPanItemManagerComponent::Server_InventoryToEquipment_Implementation(UPanInventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos, UPanEquipmentManagerComponent* ToEquipmentManager, EEquipmentSlotType ToEquipmentSlotType)
{
	if (HasAuthority() == false)
		return;
	
	if (FromInventoryManager == nullptr || ToEquipmentManager == nullptr)
		return;

	if (IsAllowedComponent(FromInventoryManager) == false ||  IsAllowedComponent(ToEquipmentManager) == false)
		return;

	int32 MovableCount = ToEquipmentManager->CanMoveOrMergeEquipment(FromInventoryManager, FromItemSlotPos, ToEquipmentSlotType);
	if (MovableCount > 0)
	{
		UPanItemInstance* RemovedItemInstance = FromInventoryManager->RemoveItem_Unsafe(FromItemSlotPos, MovableCount);
		ToEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, RemovedItemInstance, MovableCount);
	}
	else
	{
		FIntPoint ToItemSlotPos;
		if (ToEquipmentManager->CanSwapEquipment(FromInventoryManager, FromItemSlotPos, ToEquipmentSlotType, ToItemSlotPos))
		{
			const int32 FromItemCount = FromInventoryManager->GetItemCount(FromItemSlotPos);
			const int32 ToItemCount = ToEquipmentManager->GetItemCount(ToEquipmentSlotType);
			
			UPanItemInstance* RemovedItemInstanceFrom = FromInventoryManager->RemoveItem_Unsafe(FromItemSlotPos, FromItemCount);
			UPanItemInstance* RemovedItemInstanceTo = ToEquipmentManager->RemoveEquipment_Unsafe(ToEquipmentSlotType, ToItemCount);
			FromInventoryManager->AddItem_Unsafe(ToItemSlotPos, RemovedItemInstanceTo, ToItemCount);
			ToEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, RemovedItemInstanceFrom, FromItemCount);
		}
	}
}

void UPanItemManagerComponent::Server_EquipmentToInventory_Implementation(UPanEquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType, UPanInventoryManagerComponent* ToInventoryManager, const FIntPoint& ToItemSlotPos)
{
	if (HasAuthority() == false)
		return;
	
	if (FromEquipmentManager == nullptr || ToInventoryManager == nullptr)
		return;

	if (IsAllowedComponent(FromEquipmentManager) == false ||  IsAllowedComponent(ToInventoryManager) == false)
		return;

	int32 MovableCount = ToInventoryManager->CanMoveOrMergeItem(FromEquipmentManager, FromEquipmentSlotType, ToItemSlotPos);
	if (MovableCount > 0)
	{
		UPanItemInstance* RemovedItemInstance = FromEquipmentManager->RemoveEquipment_Unsafe(FromEquipmentSlotType, MovableCount);
		ToInventoryManager->AddItem_Unsafe(ToItemSlotPos, RemovedItemInstance, MovableCount);
	}
}

void UPanItemManagerComponent::Server_InventoryToInventory_Implementation(UPanInventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos, UPanInventoryManagerComponent* ToInventoryManager, const FIntPoint& ToItemSlotPos)
{
	if (HasAuthority() == false)
		return;
	
	if (FromInventoryManager == nullptr || ToInventoryManager == nullptr)
		return;

	if (IsAllowedComponent(FromInventoryManager) == false ||  IsAllowedComponent(ToInventoryManager) == false)
		return;

	if (FromInventoryManager == ToInventoryManager && FromItemSlotPos == ToItemSlotPos)
		return;

	int32 MovableCount = ToInventoryManager->CanMoveOrMergeItem(FromInventoryManager, FromItemSlotPos, ToItemSlotPos);
	if (MovableCount > 0)
	{
		UPanItemInstance* RemovedItemInstance = FromInventoryManager->RemoveItem_Unsafe(FromItemSlotPos, MovableCount);
		ToInventoryManager->AddItem_Unsafe(ToItemSlotPos, RemovedItemInstance, MovableCount);
	}
}

void UPanItemManagerComponent::Server_EquipmentToEquipment_Implementation(UPanEquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType, UPanEquipmentManagerComponent* ToEquipmentManager, EEquipmentSlotType ToEquipmentSlotType)
{
	if (HasAuthority() == false)
		return;
	
	if (FromEquipmentManager == nullptr || ToEquipmentManager == nullptr)
		return;

	if (IsAllowedComponent(FromEquipmentManager) == false ||  IsAllowedComponent(ToEquipmentManager) == false)
		return;

	if (FromEquipmentManager == ToEquipmentManager && FromEquipmentSlotType == ToEquipmentSlotType)
		return;

	int32 MovableCount = ToEquipmentManager->CanMoveOrMergeEquipment(FromEquipmentManager, FromEquipmentSlotType, ToEquipmentSlotType);
	if (MovableCount > 0)
	{
		UPanItemInstance* RemovedItemInstance = FromEquipmentManager->RemoveEquipment_Unsafe(FromEquipmentSlotType, MovableCount);
		ToEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, RemovedItemInstance, MovableCount);
	}
	else if (ToEquipmentManager->CanSwapEquipment(FromEquipmentManager, FromEquipmentSlotType, ToEquipmentSlotType))
	{
		const int32 FromItemCount = FromEquipmentManager->GetItemCount(FromEquipmentSlotType);
		const int32 ToItemCount = ToEquipmentManager->GetItemCount(ToEquipmentSlotType);
		
		UPanItemInstance* RemovedItemInstanceFrom = FromEquipmentManager->RemoveEquipment_Unsafe(FromEquipmentSlotType, FromItemCount);
		UPanItemInstance* RemovedItemInstanceTo = ToEquipmentManager->RemoveEquipment_Unsafe(ToEquipmentSlotType, ToItemCount);
		FromEquipmentManager->AddEquipment_Unsafe(FromEquipmentSlotType, RemovedItemInstanceTo, ToItemCount);
		ToEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, RemovedItemInstanceFrom, FromItemCount);
	}
}

void UPanItemManagerComponent::Server_QuickFromInventory_Implementation(UPanInventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos)
{
	if (HasAuthority() == false)
		return;
	
	if (FromInventoryManager == nullptr)
		return;

	if (IsAllowedComponent(FromInventoryManager) == false)
		return;
	
	UPanInventoryManagerComponent* MyInventoryManager = GetMyInventoryManager();
	UPanEquipmentManagerComponent* MyEquipmentManager = GetMyEquipmentManager();
	if (MyInventoryManager == nullptr || MyEquipmentManager == nullptr)
		return;

	if (IsAllowedComponent(MyInventoryManager) == false || IsAllowedComponent(MyEquipmentManager) == false)
		return;

	UPanItemInstance* FromItemInstance = FromInventoryManager->GetItemInstance(FromItemSlotPos);
	if (FromItemInstance == nullptr)
		return;

	if (FromItemInstance->FindFragmentByClass<UPanItemFragment_Equipable>())
	{
		// 1. [장비]
		// 1-1. [내 인벤토리] -> 내 장비 교체 -> 내 장비 장착 
		// 1-2. [다른 인벤토리] -> 내 장비 교체 -> 내 장비 장착 -> 내 인벤토리

		EEquipmentSlotType ToEquipmentSlotType;
		FIntPoint ToItemSlotPos;
		if (MyEquipmentManager->CanSwapEquipment_Quick(FromInventoryManager, FromItemSlotPos, ToEquipmentSlotType, ToItemSlotPos))
		{
			const int32 FromItemCount = FromInventoryManager->GetItemCount(FromItemSlotPos);
			const int32 ToItemCount = MyEquipmentManager->GetItemCount(ToEquipmentSlotType);
				
			UPanItemInstance* RemovedItemInstanceFrom = FromInventoryManager->RemoveItem_Unsafe(FromItemSlotPos, FromItemCount);
			UPanItemInstance* RemovedItemInstanceTo = MyEquipmentManager->RemoveEquipment_Unsafe(ToEquipmentSlotType, ToItemCount);
			FromInventoryManager->AddItem_Unsafe(ToItemSlotPos, RemovedItemInstanceTo, ToItemCount);
			MyEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, RemovedItemInstanceFrom, FromItemCount);
		}
		else
		{
			int32 MovableCount = MyEquipmentManager->CanMoveOrMergeEquipment_Quick(FromInventoryManager, FromItemSlotPos, ToEquipmentSlotType);
			if (MovableCount > 0)
			{
				UPanItemInstance* RemovedItemInstance = FromInventoryManager->RemoveItem_Unsafe(FromItemSlotPos, MovableCount);
				MyEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, RemovedItemInstance, MovableCount);
			}
			else
			{
				if (MyInventoryManager != FromInventoryManager)
				{
					TArray<FIntPoint> OutToItemSlotPoses;
					TArray<int32> OutToItemCounts;
					MovableCount = MyInventoryManager->CanMoveOrMergeItem_Quick(FromInventoryManager, FromItemSlotPos, OutToItemSlotPoses, OutToItemCounts);
					if (MovableCount > 0)
					{
						UPanItemInstance* RemovedItemInstance = FromInventoryManager->RemoveItem_Unsafe(FromItemSlotPos, MovableCount);
						for (int32 i = 0; i < OutToItemSlotPoses.Num(); i++)
						{
							MyInventoryManager->AddItem_Unsafe(OutToItemSlotPoses[i], RemovedItemInstance, OutToItemCounts[i]);
						}
						return;
					}
				}
			}
		}
	}
	else
	{
		// 2. [일반 아이템]
		// 2-1. [내 인벤토리] -> X
		// 2-2. [다른 인벤토리] -> 내 인벤토리

		if (MyInventoryManager != FromInventoryManager)
		{
			TArray<FIntPoint> ToItemSlotPoses;
			TArray<int32> ToItemCounts;
			
			int32 MovableItemCount = MyInventoryManager->CanMoveOrMergeItem_Quick(FromInventoryManager, FromItemSlotPos, ToItemSlotPoses, ToItemCounts);
			if (MovableItemCount > 0)
			{
				UPanItemInstance* RemovedItemInstance = FromInventoryManager->RemoveItem_Unsafe(FromItemSlotPos, MovableItemCount);
				for (int32 i = 0; i < ToItemSlotPoses.Num(); i++)
				{
					MyInventoryManager->AddItem_Unsafe(ToItemSlotPoses[i], RemovedItemInstance, ToItemCounts[i]);
				}
			}
		}
	}
}

void UPanItemManagerComponent::Server_QuickFromEquipment_Implementation(UPanEquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType)
{
	if (HasAuthority() == false)
		return;
	
	if (FromEquipmentManager == nullptr || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return;

	if (IsAllowedComponent(FromEquipmentManager) == false)
		return;

	// 1. [내 장비창] -> 내 인벤토리
	// 2. [다른 장비창] -> 내 장비 교체 -> 내 장비 장착 -> 내 인벤토리 

	UPanInventoryManagerComponent* MyInventoryManager = GetMyInventoryManager();
	UPanEquipmentManagerComponent* MyEquipmentManager = GetMyEquipmentManager();
	if (MyInventoryManager == nullptr || MyEquipmentManager == nullptr)
		return;

	if (IsAllowedComponent(MyInventoryManager) == false || IsAllowedComponent(MyEquipmentManager) == false)
		return;

	if (MyEquipmentManager == FromEquipmentManager)
	{
		TArray<FIntPoint> ToItemSlotPoses;
		TArray<int32> ToItemCounts;
		
		int32 MovableCount = MyInventoryManager->CanMoveOrMergeItem_Quick(FromEquipmentManager, FromEquipmentSlotType, ToItemSlotPoses, ToItemCounts);
		if (MovableCount > 0)
		{
			UPanItemInstance* RemovedItemInstance = FromEquipmentManager->RemoveEquipment_Unsafe(FromEquipmentSlotType, MovableCount);
			for (int32 i = 0; i < ToItemSlotPoses.Num(); i++)
			{
				MyInventoryManager->AddItem_Unsafe(ToItemSlotPoses[i], RemovedItemInstance, ToItemCounts[i]);
			}
		}
	}
	else
	{
		EEquipmentSlotType ToEquipmentSlotType;
		if (MyEquipmentManager->CanSwapEquipment_Quick(FromEquipmentManager, FromEquipmentSlotType, ToEquipmentSlotType))
		{
			const int32 FromItemCount = FromEquipmentManager->GetItemCount(FromEquipmentSlotType);
			const int32 ToItemCount = MyEquipmentManager->GetItemCount(ToEquipmentSlotType);
					
			UPanItemInstance* RemovedItemInstanceFrom = FromEquipmentManager->RemoveEquipment_Unsafe(FromEquipmentSlotType, FromItemCount);
			UPanItemInstance* RemovedItemInstanceTo = MyEquipmentManager->RemoveEquipment_Unsafe(ToEquipmentSlotType, ToItemCount);
			FromEquipmentManager->AddEquipment_Unsafe(FromEquipmentSlotType, RemovedItemInstanceTo, ToItemCount);
			MyEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, RemovedItemInstanceFrom, FromItemCount);
		}
		else
		{
			int32 MovableCount = MyEquipmentManager->CanMoveOrMergeEquipment_Quick(FromEquipmentManager, FromEquipmentSlotType, ToEquipmentSlotType);
			if (MovableCount > 0)
			{
				UPanItemInstance* RemovedItemInstance = FromEquipmentManager->RemoveEquipment_Unsafe(FromEquipmentSlotType, MovableCount);
				MyEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, RemovedItemInstance, MovableCount);
			}
			else
			{
				TArray<FIntPoint> ToItemSlotPoses;
				TArray<int32> ToItemCounts;

				MovableCount = MyInventoryManager->CanMoveOrMergeItem_Quick(FromEquipmentManager, FromEquipmentSlotType, ToItemSlotPoses, ToItemCounts);
				if (MovableCount > 0)
				{
					UPanItemInstance* RemovedItemInstance = FromEquipmentManager->RemoveEquipment_Unsafe(FromEquipmentSlotType, MovableCount);
					for (int32 i = 0; i < ToItemSlotPoses.Num(); i++)
					{
						MyInventoryManager->AddItem_Unsafe(ToItemSlotPoses[i], RemovedItemInstance, ToItemCounts[i]);
					}
				}
			}
		}
	}
}

void UPanItemManagerComponent::Server_DropItemFromInventory_Implementation(UPanInventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos)
{
	if (HasAuthority() == false)
		return;

	if (FromInventoryManager == nullptr)
		return;

	if (IsAllowedComponent(FromInventoryManager) == false)
		return;

	UPanItemInstance* FromItemInstance = FromInventoryManager->GetItemInstance(FromItemSlotPos);
	if (FromItemInstance == nullptr)
		return;

	int32 FromItemCount = FromInventoryManager->GetItemCount(FromItemSlotPos);
	if (FromItemCount <= 0)
		return;
	
	//if (TryDropItem(FromItemInstance, FromItemCount))
	{
		FromInventoryManager->RemoveItem_Unsafe(FromItemSlotPos, FromItemCount);
	}
}

void UPanItemManagerComponent::Server_DropItemFromEquipment_Implementation(UPanEquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType)
{
	if (HasAuthority() == false)
		return;
	
	if (FromEquipmentManager == nullptr || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return;

	if (IsAllowedComponent(FromEquipmentManager) == false)
		return;

	UPanItemInstance* FromItemInstance = FromEquipmentManager->GetItemInstance(FromEquipmentSlotType);
	if (FromItemInstance == nullptr)
		return;

	int32 FromItemCount = FromEquipmentManager->GetItemCount(FromEquipmentSlotType);
	if (FromItemCount <= 0)
		return;
	
	//if (TryDropItem(FromItemInstance, FromItemCount))
	{
		FromEquipmentManager->RemoveEquipment_Unsafe(FromEquipmentSlotType, FromItemCount);
	}
}

/*
bool UPanItemManagerComponent::TryPickItem(APanPickupableItemBase* PickupableItemActor)
{
	if (HasAuthority() == false)
		return false;

	if (IsValid(PickupableItemActor) == false)
		return false;

	UPanInventoryManagerComponent* MyInventoryManager = GetMyInventoryManager();
	UPanEquipmentManagerComponent* MyEquipmentManager = GetMyEquipmentManager();
	if (MyInventoryManager == nullptr || MyEquipmentManager == nullptr)
		return false;

	if (IsAllowedComponent(MyInventoryManager) == false || IsAllowedComponent(MyEquipmentManager) == false)
		return false;

	const FPanPickupInfo& PickupInfo = PickupableItemActor->GetPickupInfo();
	const FPanPickupInstance& PickupInstance = PickupInfo.PickupInstance;
	const FPanPickupTemplate& PickupTemplate = PickupInfo.PickupTemplate;

	int32 ItemTemplateID = 0;
	EItemRarity ItemRarity = EItemRarity::Count;
	int32 ItemCount = 0;
	UPanItemInstance* ItemInstance = nullptr;
	
	if (PickupInstance.ItemInstance)
	{
		if (PickupInstance.ItemCount <= 0)
			return false;

		ItemTemplateID = PickupInstance.ItemInstance->GetItemTemplateID();
		ItemRarity = PickupInstance.ItemInstance->GetItemRarity();
		ItemCount = PickupInstance.ItemCount;
		ItemInstance = PickupInstance.ItemInstance;
	}
	else if (PickupTemplate.ItemTemplateClass)
	{
		if (PickupTemplate.ItemCount <= 0 || PickupTemplate.ItemRarity == EItemRarity::Count)
			return false;
		
		ItemTemplateID = UPanItemData::Get().FindItemTemplateIDByClass(PickupTemplate.ItemTemplateClass);
		ItemRarity = PickupTemplate.ItemRarity;
		ItemCount = PickupTemplate.ItemCount;
	}

	EEquipmentSlotType ToEquipmentSlotType;
	int32 MovableCount = MyEquipmentManager->CanMoveOrMergeEquipment_Quick(ItemTemplateID, ItemRarity, ItemCount, ToEquipmentSlotType);
	if (MovableCount == ItemCount)
	{
		if (ItemInstance == nullptr)
		{
			ItemInstance = NewObject<UPanItemInstance>();
			ItemInstance->Init(ItemTemplateID, ItemRarity);
		}
		
		MyEquipmentManager->AddEquipment_Unsafe(ToEquipmentSlotType, ItemInstance, MovableCount);
		
		PickupableItemActor->Destroy();
		return true;
	}
	else
	{
		TArray<FIntPoint> ToItemSlotPoses;
		TArray<int32> ToItemCounts;
			
		MovableCount = MyInventoryManager->CanAddItem(ItemTemplateID, ItemRarity, ItemCount, ToItemSlotPoses, ToItemCounts);
		if (MovableCount == ItemCount)
		{
			if (ItemInstance == nullptr)
			{
				ItemInstance = NewObject<UPanItemInstance>();
				ItemInstance->Init(ItemTemplateID, ItemRarity);
			}
			
			for (int32 i = 0; i < ToItemSlotPoses.Num(); i++)
			{
				MyInventoryManager->AddItem_Unsafe(ToItemSlotPoses[i], ItemInstance, ToItemCounts[i]);
			}
			
			PickupableItemActor->Destroy();
			return true;
		}
	}
	
	return false;
}


bool UPanItemManagerComponent::TryDropItem(UPanItemInstance* FromItemInstance, int32 FromItemCount)
{
	if (HasAuthority() == false)
		return false;

	if (FromItemInstance == nullptr || FromItemCount <= 0)
		return false;

	AController* Controller = Cast<AController>(GetOwner());
	ACharacter* Character = Controller ? Cast<ACharacter>(Controller->GetPawn()) : Cast<ACharacter>(GetOwner());
	if (Character == nullptr)
		return false;
	
	const float MaxDistance = 100.f;
	const int32 MaxTryCount = 5.f;
	float HalfRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius() / 2.f;
	float QuarterHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2.f;
	TArray<AActor*> ActorsToIgnore = { Character };

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	for (int32 i = 0; i < MaxTryCount; i++)
	{
		FHitResult HitResult;
		FVector2D RandPoint = FMath::RandPointInCircle(MaxDistance);
		FVector TraceStartLocation = Character->GetCapsuleComponent()->GetComponentLocation();
		FVector TraceEndLocation = TraceStartLocation + FVector(RandPoint.X, RandPoint.Y, 0.f);
		
		if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), TraceStartLocation, TraceEndLocation, HalfRadius, QuarterHeight, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true))
			continue;
		
		TSubclassOf<APanPickupableItemBase> PickupableItemBaseClass = ULyraAssetManager::Get().GetSubclassByName<APanPickupableItemBase>("PickupableItemBaseClass");
		APanPickupableItemBase* PickupableItemActor = GetWorld()->SpawnActor<APanPickupableItemBase>(PickupableItemBaseClass, TraceEndLocation, FRotator::ZeroRotator, SpawnParameters);
		if (PickupableItemActor == nullptr)
			continue;
		
		FPanPickupInfo PickupInfo;
		PickupInfo.PickupInstance.ItemInstance = FromItemInstance;
		PickupInfo.PickupInstance.ItemCount = FromItemCount;
		PickupableItemActor->SetPickupInfo(PickupInfo);
		return true;
	}
	
	return false;
}
*/

void UPanItemManagerComponent::AddAllowedComponent(UActorComponent* ActorComponent)
{
	AllowedComponents.Add(ActorComponent);
}

void UPanItemManagerComponent::RemoveAllowedComponent(UActorComponent* ActorComponent)
{
	AllowedComponents.Remove(ActorComponent);
}

bool UPanItemManagerComponent::IsAllowedComponent(UActorComponent* ActorComponent) const
{
	return AllowedComponents.Contains(ActorComponent);
}

UPanInventoryManagerComponent* UPanItemManagerComponent::GetMyInventoryManager() const
{
	UPanInventoryManagerComponent* MyInventoryManager = nullptr;
	
	if (AController* Controller = Cast<AController>(GetOwner()))
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			MyInventoryManager = Pawn->GetComponentByClass<UPanInventoryManagerComponent>();
		}
	}

	return MyInventoryManager;
}

UPanEquipmentManagerComponent* UPanItemManagerComponent::GetMyEquipmentManager() const
{
	UPanEquipmentManagerComponent* MyEquipmentManager = nullptr;
	
	if (AController* Controller = Cast<AController>(GetOwner()))
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			MyEquipmentManager = Pawn->GetComponentByClass<UPanEquipmentManagerComponent>();
		}
	}

	return MyEquipmentManager;
}
