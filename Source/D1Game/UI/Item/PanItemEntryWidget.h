#pragma once

#include "Blueprint/UserWidget.h"
#include "PanItemEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UPanItemInstance;
class UPanItemDragWidget;
class UPanItemHoverWidget;

UCLASS()
class UPanItemEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanItemEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	void RefreshWidgetOpacity(bool bClearlyVisible);

public:
	void RefreshUI(UPanItemInstance* NewItemInstance, int32 NewItemCount);
	void RefreshItemCount(int32 NewItemCount);
	UPanItemInstance* GetItemInstance() const { return ItemInstance; }

protected:
	UPROPERTY()
	TObjectPtr<UPanItemInstance> ItemInstance;

	UPROPERTY()
	int32 ItemCount = 0;
	
	UPROPERTY()
	TObjectPtr<UPanItemHoverWidget> HoverWidget;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_RarityCover;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Hover;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Count;
};
