#pragma once

#include "PanDefine.h"
#include "GameplayTagContainer.h"
#include "PanUIData.generated.h"

class UImage;
class UPanSkillStatHoverWidget;
class UPanInventoryValidWidget;
class UPanItemDragWidget;
class UPanItemHoverWidget;
class UPanEquipmentEntryWidget;
class UPanInventorySlotWidget;
class UPanInventoryEntryWidget;

USTRUCT(BlueprintType)
struct FPanItemRarityInfoEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta=(HideAlphaChannel))
	FColor Color = FColor::Black;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EntryTexture;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> HoverTexture;
};

USTRUCT(BlueprintType)
struct FPanUIInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly)
	FText Title;

	UPROPERTY(EditDefaultsOnly)
	FText Content;
};

UCLASS(BlueprintType, Const, meta=(DisplayName="Pan UI Data"))
class UPanUIData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	static const UPanUIData& Get();
	
public:
	UTexture2D* GetEntryRarityTexture(EItemRarity ItemRarity) const;
	UTexture2D* GetHoverRarityTexture(EItemRarity ItemRarity) const;
	FColor GetRarityColor(EItemRarity ItemRarity) const;

	const FPanUIInfo& GetTagUIInfo(FGameplayTag Tag) const;

public:
	UPROPERTY(EditDefaultsOnly)
	FIntPoint UnitInventorySlotSize = FIntPoint::ZeroValue;
	
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanItemDragWidget> DragWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanItemHoverWidget> ItemHoverWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanSkillStatHoverWidget> SkillStatHoverWidget;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanEquipmentEntryWidget> EquipmentEntryWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanInventorySlotWidget> InventorySlotWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanInventoryEntryWidget> InventoryEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPanInventoryValidWidget> InventoryValidWidgetClass;
	
private:
	UPROPERTY(EditDefaultsOnly, meta=(ArraySizeEnum="EItemRarity"))
	FPanItemRarityInfoEntry RarityInfoEntries[(int32)EItemRarity::Count];

	UPROPERTY(EditDefaultsOnly, meta=(DisplayName="Tag UI Infos"))
	TMap<FGameplayTag, FPanUIInfo> TagUIInfos;
};
