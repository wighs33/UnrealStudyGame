#pragma once

#include "Components/ControllerComponent.h"
#include "PanItemManagerComponent.generated.h"

class UPanItemInstance;
class APanPickupableItemBase;
class UPanEquipmentManagerComponent;
class UPanInventoryManagerComponent;

// 모든 아이템을 중앙에서 관리하고 실제로 아이템의 이동을 관장한다 (컨트롤러의 컴포넌트)
UCLASS()
class UPanItemManagerComponent : public UControllerComponent
{
	GENERATED_BODY()
	
public:
	UPanItemManagerComponent(const FObjectInitializer& ObjectInitializer);
	
public:
	// 소실되면 안 되기에 Reliable
	// 아이템의 이동 (Inventory -> Equipment)
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_InventoryToEquipment(UPanInventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos, UPanEquipmentManagerComponent* ToEquipmentManager, EEquipmentSlotType ToEquipmentSlotType);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_EquipmentToInventory(UPanEquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType, UPanInventoryManagerComponent* ToInventoryManager, const FIntPoint& ToItemSlotPos);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_InventoryToInventory(UPanInventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos, UPanInventoryManagerComponent* ToInventoryManager, const FIntPoint& ToItemSlotPos);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_EquipmentToEquipment(UPanEquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType, UPanEquipmentManagerComponent* ToEquipmentManager, EEquipmentSlotType ToEquipmentSlotType);
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_QuickFromInventory(UPanInventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_QuickFromEquipment(UPanEquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType);
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropItemFromInventory(UPanInventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropItemFromEquipment(UPanEquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType);

public:
	/*UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryPickItem(APanPickupableItemBase* PickupableItemActor);*/

	/*UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryDropItem(UPanItemInstance* FromItemInstance, int32 FromItemCount);*/
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddAllowedComponent(UActorComponent* ActorComponent);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemoveAllowedComponent(UActorComponent* ActorComponent);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure)
	bool IsAllowedComponent(UActorComponent* ActorComponent) const;

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPanInventoryManagerComponent* GetMyInventoryManager() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPanEquipmentManagerComponent* GetMyEquipmentManager() const;
	
private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UActorComponent>> AllowedComponents;
};
