#pragma once

#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PanInventoryStatWidget.generated.h"

class UPanSkillStatHoverWidget;

UCLASS()
class UPanInventoryStatWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanInventoryStatWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeDestruct() override;
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag StatTag;
	
protected:
	UPROPERTY()
	TObjectPtr<UPanSkillStatHoverWidget> HoverWidget;
};
