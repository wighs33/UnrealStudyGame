#include "PanInventorySlotsWidget.h"

#include "AbilitySystemComponent.h"
#include "PanInventoryEntryWidget.h"
#include "PanInventorySlotWidget.h"
#include "PanInventoryValidWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Data/PanItemData.h"
#include "Data/PanUIData.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Item/PanItemInstance.h"
#include "Item/PanItemTemplate.h"
#include "Item/Managers/PanEquipmentManagerComponent.h"
#include "Item/Managers/PanInventoryManagerComponent.h"
#include "Item/Managers/PanItemManagerComponent.h"
#include "UI/Item/PanItemDragDrop.h"
#include "UI/Item/Equipment/PanEquipmentEntryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanInventorySlotsWidget)

UPanInventorySlotsWidget::UPanInventorySlotsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleText = FText::FromString(TEXT("Title"));
}

void UPanInventorySlotsWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	Text_Title->SetText(TitleText);
}

void UPanInventorySlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageListenerHandle = MessageSubsystem.RegisterListener(MessageChannelTag, this, &ThisClass::ConstructUI);
}

void UPanInventorySlotsWidget::NativeDestruct()
{
	DestructUI();
	
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(MessageListenerHandle);
	
	Super::NativeDestruct();
}

bool UPanInventorySlotsWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	
	UPanItemDragDrop* DragDrop = Cast<UPanItemDragDrop>(InOperation);
	if (DragDrop == nullptr)
		return false;

	FIntPoint UnitInventorySlotSize = UPanUIData::Get().UnitInventorySlotSize;
	
	FVector2D MouseWidgetPos = GetSlotContainerGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	FVector2D ToWidgetPos = MouseWidgetPos - DragDrop->DeltaWidgetPos;
	FIntPoint ToItemSlotPos = FIntPoint(ToWidgetPos.X / UnitInventorySlotSize.X, ToWidgetPos.Y / UnitInventorySlotSize.Y);

	if (PrevDragOverSlotPos == ToItemSlotPos)
		return true;
	
	PrevDragOverSlotPos = ToItemSlotPos;
	
	UPanItemEntryWidget* FromEntryWidget = DragDrop->FromEntryWidget;
	UPanItemInstance* FromItemInstance = FromEntryWidget->GetItemInstance();
	if (FromItemInstance == nullptr)
		return false;
	
	const UPanItemTemplate& FromItemTemplate = UPanItemData::Get().FindItemTemplateByID(FromItemInstance->GetItemTemplateID());
	const FIntPoint& FromItemSlotCount = FromItemTemplate.SlotCount;

	int32 MovableCount = 0;
	if (UPanInventoryManagerComponent* FromInventoryManager = DragDrop->FromInventoryManager)
	{
		MovableCount = InventoryManager->CanMoveOrMergeItem(FromInventoryManager, DragDrop->FromItemSlotPos, ToItemSlotPos);
	}
	else if (UPanEquipmentManagerComponent* FromEquipmentManager = DragDrop->FromEquipmentManager)
	{
		MovableCount = InventoryManager->CanMoveOrMergeItem(FromEquipmentManager, DragDrop->FromEquipmentSlotType, ToItemSlotPos);
	}
	
	ResetValidSlots();

	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();
	
	const FIntPoint StartSlotPos = FIntPoint(FMath::Max(0, ToItemSlotPos.X), FMath::Max(0, ToItemSlotPos.Y));
	const FIntPoint EndSlotPos   = FIntPoint(FMath::Min(ToItemSlotPos.X + FromItemSlotCount.X, InventorySlotCount.X),
											 FMath::Min(ToItemSlotPos.Y + FromItemSlotCount.Y, InventorySlotCount.Y));

	ESlotState SlotState = (MovableCount > 0) ? ESlotState::Valid : ESlotState::Invalid;
	for (int32 y = StartSlotPos.Y; y < EndSlotPos.Y; y++)
	{
		for (int32 x = StartSlotPos.X; x < EndSlotPos.X; x++)
		{
			int32 Index = y * InventorySlotCount.X + x;
			if (UPanInventoryValidWidget* ValidWidget = ValidWidgets[Index])
			{
				ValidWidget->ChangeSlotState(SlotState);
			}
		}
	}
	return true;
}

void UPanInventorySlotsWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	FinishDrag();
}

bool UPanInventorySlotsWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	FinishDrag();

	FIntPoint UnitInventorySlotSize = UPanUIData::Get().UnitInventorySlotSize;
	
	UPanItemDragDrop* DragDrop = Cast<UPanItemDragDrop>(InOperation);
	check(DragDrop);

	UPanItemEntryWidget* FromEntryWidget = DragDrop->FromEntryWidget;
	FromEntryWidget->RefreshWidgetOpacity(true);
	
	FVector2D MouseWidgetPos = GetSlotContainerGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	FVector2D ToWidgetPos = MouseWidgetPos - DragDrop->DeltaWidgetPos;
	FIntPoint ToItemSlotPos = FIntPoint(ToWidgetPos.X / UnitInventorySlotSize.X, ToWidgetPos.Y / UnitInventorySlotSize.Y);

	DragDrop->ToInventoryManager = InventoryManager;
    DragDrop->ToItemSlotPos = ToItemSlotPos;

	UPanItemManagerComponent* ItemManager = GetOwningPlayer()->FindComponentByClass<UPanItemManagerComponent>();
	check(ItemManager);
	
	if (UPanInventoryManagerComponent* FromInventoryManager = DragDrop->FromInventoryManager)
	{
		ItemManager->Server_InventoryToInventory(FromInventoryManager, DragDrop->FromItemSlotPos, InventoryManager, ToItemSlotPos);
	}
	else if (UPanEquipmentManagerComponent* FromEquipmentManager = DragDrop->FromEquipmentManager)
	{
		ItemManager->Server_EquipmentToInventory(FromEquipmentManager, DragDrop->FromEquipmentSlotType, InventoryManager, ToItemSlotPos);
	}
	return true;
}

void UPanInventorySlotsWidget::ConstructUI(FGameplayTag Channel, const FInventoryInitializeMessage& Message)
{
	if (Message.InventoryManager == nullptr)
		return;
	
	InventoryManager = Message.InventoryManager;
	
	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();
	const int32 InventorySlotNum = InventorySlotCount.X * InventorySlotCount.Y;
	SlotWidgets.SetNum(InventorySlotNum);
	ValidWidgets.SetNum(InventorySlotNum);
	EntryWidgets.SetNum(InventorySlotNum);
	
	for (int32 y = 0; y < InventorySlotCount.Y; y++)
	{
		for (int32 x = 0; x < InventorySlotCount.X; x++)
		{
			TSubclassOf<UPanInventorySlotWidget> SlotWidgetClass = UPanUIData::Get().InventorySlotWidgetClass;
			UPanInventorySlotWidget* SlotWidget = CreateWidget<UPanInventorySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
			SlotWidgets[y * InventorySlotCount.X + x] = SlotWidget;
			GridPanel_Slots->AddChildToUniformGrid(SlotWidget, y, x);

			TSubclassOf<UPanInventoryValidWidget> ValidWidgetClass = UPanUIData::Get().InventoryValidWidgetClass;
			UPanInventoryValidWidget* ValidWidget = CreateWidget<UPanInventoryValidWidget>(GetOwningPlayer(), ValidWidgetClass);
			ValidWidgets[y * InventorySlotCount.X + x] = ValidWidget;
			GridPanel_ValidSlots->AddChildToUniformGrid(ValidWidget, y, x);
		}
	}
	
	const TArray<FPanInventoryEntry>& Entries = InventoryManager->GetAllEntries();
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		const FPanInventoryEntry& Entry = Entries[i];
		if (UPanItemInstance* ItemInstance = Entry.GetItemInstance())
		{
			FIntPoint ItemSlotPos = FIntPoint(i % InventorySlotCount.X, i / InventorySlotCount.X);
			OnInventoryEntryChanged(ItemSlotPos, ItemInstance, Entry.GetItemCount());
		}
	}
	EntryChangedDelegateHandle = InventoryManager->OnInventoryEntryChanged.AddUObject(this, &ThisClass::OnInventoryEntryChanged);
}

void UPanInventorySlotsWidget::DestructUI()
{
	InventoryManager->OnInventoryEntryChanged.Remove(EntryChangedDelegateHandle);
	EntryChangedDelegateHandle.Reset();

	CanvasPanel_Entries->ClearChildren();
	EntryWidgets.Reset();
	
	GridPanel_Slots->ClearChildren();
	SlotWidgets.Reset();
}

void UPanInventorySlotsWidget::ResetValidSlots()
{
	for (UPanInventoryValidWidget* ValidWidget : ValidWidgets)
	{
		if (ValidWidget)
		{
			ValidWidget->ChangeSlotState(ESlotState::Default);
		}
	}
}

void UPanInventorySlotsWidget::FinishDrag()
{
	ResetValidSlots();
	PrevDragOverSlotPos = FIntPoint(-1, -1);
}

void UPanInventorySlotsWidget::OnInventoryEntryChanged(const FIntPoint& InItemSlotPos, UPanItemInstance* InItemInstance, int32 InItemCount)
{
	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();
	int32 SlotIndex = InItemSlotPos.Y * InventorySlotCount.X + InItemSlotPos.X;

	if (UPanInventoryEntryWidget* EntryWidget = EntryWidgets[SlotIndex])
	{
		UPanItemInstance* ItemInstance = EntryWidget->GetItemInstance();
		if (ItemInstance && ItemInstance == InItemInstance)
		{
			EntryWidget->RefreshItemCount(InItemCount);
			return;
		}
		
		CanvasPanel_Entries->RemoveChild(EntryWidget);
		EntryWidgets[SlotIndex] = nullptr;
	}
	
	if (InItemInstance)
	{
		FIntPoint UnitInventorySlotSize = UPanUIData::Get().UnitInventorySlotSize;
		
		TSubclassOf<UPanInventoryEntryWidget> EntryWidgetClass = UPanUIData::Get().InventoryEntryWidgetClass;
		UPanInventoryEntryWidget* EntryWidget = CreateWidget<UPanInventoryEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		EntryWidgets[SlotIndex] = EntryWidget;
		
		EntryWidget->Init(this, InItemInstance, InItemCount);
		
		UCanvasPanelSlot* CanvasPanelSlot = CanvasPanel_Entries->AddChildToCanvas(EntryWidget);
		CanvasPanelSlot->SetAutoSize(true);
		CanvasPanelSlot->SetPosition(FVector2D(InItemSlotPos.X * UnitInventorySlotSize.X, InItemSlotPos.Y * UnitInventorySlotSize.Y));
	}
}

const FGeometry& UPanInventorySlotsWidget::GetSlotContainerGeometry() const
{
	return Overlay_Slots->GetCachedGeometry();
}
