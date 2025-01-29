#pragma once

#include "Blueprint/UserWidget.h"
#include "PanItemDragWidget.generated.h"

class UTexture2D;
class USizeBox;
class UImage;
class UTextBlock;

UCLASS()
class UPanItemDragWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanItemDragWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(const FVector2D& InWidgetSize, UTexture2D* InItemIcon, int32 InItemCount);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Count;
};
