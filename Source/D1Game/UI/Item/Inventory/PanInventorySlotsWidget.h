#pragma once

#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PanInventorySlotsWidget.generated.h"

class UPanInventoryValidWidget;
class UOverlay;
class UTextBlock;
class UPanInventoryEntryWidget;
class UPanInventorySlotWidget;
class UPanInventoryManagerComponent;
class UUniformGridPanel;
class UCanvasPanel;
class UPanItemInstance;

USTRUCT(BlueprintType)
struct FInventoryInitializeMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UPanInventoryManagerComponent> InventoryManager;
};

UCLASS()
class UPanInventorySlotsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanInventorySlotsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	void ConstructUI(FGameplayTag Channel, const FInventoryInitializeMessage& Message);
	void DestructUI();
	
	void ResetValidSlots();
	void FinishDrag();
	void OnInventoryEntryChanged(const FIntPoint& ItemSlotPos, UPanItemInstance* ItemInstance, int32 InItemCount);

public:
	UPanInventoryManagerComponent* GetInventoryManager() const { return InventoryManager; }
	const FGeometry& GetSlotContainerGeometry() const;

public:
	UPROPERTY(EditAnywhere, meta=(Categories="Message"))
	FGameplayTag MessageChannelTag;

	UPROPERTY(EditAnywhere)
	FText TitleText;

private:
	UPROPERTY()
	TArray<TObjectPtr<UPanInventorySlotWidget>> SlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UPanInventoryEntryWidget>> EntryWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UPanInventoryValidWidget>> ValidWidgets;

	UPROPERTY()
	TObjectPtr<UPanInventoryManagerComponent> InventoryManager;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Title;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> Overlay_Slots;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel_Slots;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Entries;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel_ValidSlots;
	
private:
	FDelegateHandle EntryChangedDelegateHandle;
	FIntPoint PrevDragOverSlotPos = FIntPoint(-1, -1);
	FGameplayMessageListenerHandle MessageListenerHandle;
};
